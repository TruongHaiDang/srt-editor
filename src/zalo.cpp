#include "zalo.h"

#include <curl/curl.h>

#include <cmath>
#include <chrono>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace
{
constexpr const char* kSynthesizePath = "/v1/tts/synthesize";
constexpr const char* kApiKeyHeaderPrefix = "apikey: ";
constexpr const char* kFormContentTypeHeader =
    "Content-Type: application/x-www-form-urlencoded";
constexpr const char* kDefaultProtocol = "https";
constexpr long kFollowRedirects = 1L;
constexpr double kMinimumSpeed = 0.8;
constexpr double kMaximumSpeed = 1.2;

size_t writeResponseCallback(char* contents, size_t size, size_t nmemb, void* user_data)
{
    if (size != 0 && nmemb > std::numeric_limits<size_t>::max() / size) {
        return 0;
    }

    const size_t byte_count = size * nmemb;
    auto* response_body = static_cast<std::string*>(user_data);
    response_body->append(contents, byte_count);
    return byte_count;
}

void ensureCurlGlobalInitialized()
{
    static const bool initialized = []() {
        const CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (code != CURLE_OK) {
            throw std::runtime_error(
                std::string("Cannot initialize libcurl: ") + curl_easy_strerror(code)
            );
        }
        return true;
    }();

    (void)initialized;
}

using CurlHandle = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;
using CurlHeaders = std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)>;

CurlHeaders appendHeader(CurlHeaders headers, const std::string& header)
{
    curl_slist* updated_headers = curl_slist_append(headers.get(), header.c_str());
    if (updated_headers == nullptr) {
        throw std::runtime_error("Cannot allocate curl request header.");
    }

    headers.release();
    return CurlHeaders(updated_headers, curl_slist_free_all);
}

void checkCurlOption(CURLcode code, const std::string& context)
{
    if (code != CURLE_OK) {
        throw std::runtime_error(context + ": " + curl_easy_strerror(code));
    }
}

std::string escapeFormValue(CURL* curl, const std::string& value)
{
    if (value.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("Zalo form value is too large.");
    }

    char* escaped = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.size()));
    if (escaped == nullptr) {
        throw std::runtime_error("Cannot URL-encode Zalo request value.");
    }

    std::string result(escaped);
    curl_free(escaped);
    return result;
}

std::string buildFormBody(CURL* curl, const ZaloTextToSpeechRequest& request)
{
    std::ostringstream speed;
    speed << std::setprecision(15) << request.speed;

    return "input=" + escapeFormValue(curl, request.text) +
        "&speaker_id=" + std::to_string(request.speaker_id) +
        "&speed=" + escapeFormValue(curl, speed.str()) +
        "&encode_type=" + std::to_string(static_cast<int>(request.encoding));
}

void validateRequest(const ZaloTextToSpeechRequest& request)
{
    if (request.text.empty()) {
        throw std::invalid_argument("Zalo text-to-speech text cannot be empty.");
    }
    if (request.speaker_id <= 0) {
        throw std::invalid_argument("Zalo speaker ID must be greater than zero.");
    }
    if (!std::isfinite(request.speed) ||
        request.speed < kMinimumSpeed || request.speed > kMaximumSpeed) {
        throw std::invalid_argument("Zalo speed must be between 0.8 and 1.2.");
    }
    if (request.encoding != ZaloAudioEncoding::Wav &&
        request.encoding != ZaloAudioEncoding::Mp3) {
        throw std::invalid_argument("Zalo audio encoding is invalid.");
    }
}

ZaloResponse performRequest(
    CURL* curl,
    CurlHeaders& headers,
    const std::string& error_context
)
{
    std::string response_body;

    checkCurlOption(
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers.get()),
        "Cannot configure Zalo request headers"
    );
    checkCurlOption(
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponseCallback),
        "Cannot configure Zalo response callback"
    );
    checkCurlOption(
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body),
        "Cannot configure Zalo response buffer"
    );
    checkCurlOption(
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, kFollowRedirects),
        "Cannot configure Zalo redirects"
    );
    checkCurlOption(
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, kDefaultProtocol),
        "Cannot configure Zalo default protocol"
    );

    const CURLcode perform_code = curl_easy_perform(curl);
    if (perform_code != CURLE_OK) {
        throw std::runtime_error(error_context + ": " + curl_easy_strerror(perform_code));
    }

    long status_code = 0;
    checkCurlOption(
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code),
        "Cannot read Zalo response status"
    );

    return ZaloResponse{status_code, std::move(response_body)};
}
}

ZaloClient::ZaloClient(std::string api_key)
    : ZaloClient(std::move(api_key), kDefaultBaseUrl)
{
}

ZaloClient::ZaloClient(std::string api_key, std::string base_url)
    : api_key_(std::move(api_key))
    , base_url_(std::move(base_url))
{
    if (api_key_.empty()) {
        throw std::invalid_argument("Zalo API key cannot be empty.");
    }
    if (base_url_.empty()) {
        throw std::invalid_argument("Zalo base URL cannot be empty.");
    }
}

ZaloResponse ZaloClient::synthesize(const ZaloTextToSpeechRequest& request) const
{
    validateRequest(request);
    ensureCurlGlobalInitialized();

    CurlHandle curl(curl_easy_init(), curl_easy_cleanup);
    if (curl == nullptr) {
        throw std::runtime_error("Cannot create curl handle for Zalo request.");
    }

    const std::string url = buildUrl(kSynthesizePath);
    const std::string form_body = buildFormBody(curl.get(), request);
    if (form_body.size() > static_cast<size_t>(std::numeric_limits<long>::max())) {
        throw std::invalid_argument("Zalo request body is too large.");
    }

    CurlHeaders headers(nullptr, curl_slist_free_all);
    headers = appendHeader(std::move(headers), kFormContentTypeHeader);
    headers = appendHeader(std::move(headers), std::string(kApiKeyHeaderPrefix) + api_key_);

    checkCurlOption(curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str()), "Cannot configure Zalo URL");
    checkCurlOption(curl_easy_setopt(curl.get(), CURLOPT_POST, 1L), "Cannot configure Zalo POST request");
    checkCurlOption(curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, form_body.c_str()), "Cannot configure Zalo request body");
    checkCurlOption(curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(form_body.size())), "Cannot configure Zalo request size");

    return performRequest(curl.get(), headers, "Zalo text-to-speech request failed");
}

ZaloResponse ZaloClient::downloadAudio(
    const std::string& audio_url,
    int max_attempts,
    int retry_delay_ms
) const
{
    if (audio_url.empty()) {
        throw std::invalid_argument("Zalo audio URL cannot be empty.");
    }
    if (max_attempts <= 0) {
        throw std::invalid_argument("Zalo audio download attempts must be greater than zero.");
    }
    if (retry_delay_ms < 0) {
        throw std::invalid_argument("Zalo audio retry delay cannot be negative.");
    }

    ensureCurlGlobalInitialized();
    ZaloResponse response;
    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        CurlHandle curl(curl_easy_init(), curl_easy_cleanup);
        if (curl == nullptr) {
            throw std::runtime_error("Cannot create curl handle for Zalo audio download.");
        }

        CurlHeaders headers(nullptr, curl_slist_free_all);
        checkCurlOption(
            curl_easy_setopt(curl.get(), CURLOPT_URL, audio_url.c_str()),
            "Cannot configure Zalo audio URL"
        );
        response = performRequest(curl.get(), headers, "Zalo audio download failed");

        const bool audioIsPending = response.status_code == 404;
        if (!audioIsPending || attempt == max_attempts) {
            return response;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
    }

    return response;
}

std::string ZaloClient::buildUrl(const std::string& path) const
{
    if (path.empty() || path.front() != '/') {
        throw std::invalid_argument("Zalo API path must start with '/'.");
    }

    if (base_url_.back() == '/') {
        return base_url_.substr(0, base_url_.size() - 1) + path;
    }
    return base_url_ + path;
}

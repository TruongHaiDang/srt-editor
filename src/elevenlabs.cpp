#include "elevenlabs.h"

#include <curl/curl.h>

#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
constexpr long kFollowRedirects = 1L;
constexpr const char* kDefaultProtocol = "https";
constexpr const char* kJsonContentTypeHeader = "Content-Type: application/json";
constexpr const char* kApiKeyHeaderPrefix = "xi-api-key: ";
constexpr const char* kGetModelsPath = "/v1/models";
constexpr const char* kGetVoicesPath = "/v2/voices?page_size=";
constexpr const char* kTextToSpeechPathPrefix = "/v1/text-to-speech/";
constexpr int kMinimumVoicePageSize = 1;
constexpr int kMaximumVoicePageSize = 100;
constexpr double kMinimumVoiceSetting = 0.0;
constexpr double kMaximumVoiceSetting = 1.0;

size_t writeResponseCallback(char* contents, size_t size, size_t nmemb, void* user_data)
{
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

void setCurlOption(CURL* curl, CURLoption option, const char* value)
{
    const CURLcode code = curl_easy_setopt(curl, option, value);
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure curl string option: ") + curl_easy_strerror(code)
        );
    }
}

void setCurlOption(CURL* curl, CURLoption option, long value)
{
    const CURLcode code = curl_easy_setopt(curl, option, value);
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure curl numeric option: ") + curl_easy_strerror(code)
        );
    }
}

std::string escapeJsonString(const std::string& value)
{
    std::ostringstream stream;

    for (const unsigned char character : value) {
        switch (character) {
        case '"':
            stream << "\\\"";
            break;
        case '\\':
            stream << "\\\\";
            break;
        case '\b':
            stream << "\\b";
            break;
        case '\f':
            stream << "\\f";
            break;
        case '\n':
            stream << "\\n";
            break;
        case '\r':
            stream << "\\r";
            break;
        case '\t':
            stream << "\\t";
            break;
        default:
            if (character < 0x20) {
                stream << "\\u"
                       << std::hex
                       << std::setw(4)
                       << std::setfill('0')
                       << static_cast<int>(character)
                       << std::dec;
            } else {
                stream << character;
            }
            break;
        }
    }

    return stream.str();
}

void validateVoiceSetting(double value, const std::string& field_name)
{
    if (value < kMinimumVoiceSetting || value > kMaximumVoiceSetting) {
        throw std::invalid_argument(
            "ElevenLabs " + field_name + " must be between 0.0 and 1.0."
        );
    }
}

void validateTextToSpeechRequest(const ElevenLabsTextToSpeechRequest& request)
{
    if (request.voice_id.empty()) {
        throw std::invalid_argument("ElevenLabs voice ID cannot be empty.");
    }

    if (request.text.empty()) {
        throw std::invalid_argument("ElevenLabs text-to-speech text cannot be empty.");
    }

    if (request.model_id.empty()) {
        throw std::invalid_argument("ElevenLabs model ID cannot be empty.");
    }

    if (request.output_format.empty()) {
        throw std::invalid_argument("ElevenLabs output format cannot be empty.");
    }

    validateVoiceSetting(request.voice_settings.stability, "stability");
    validateVoiceSetting(request.voice_settings.similarity_boost, "similarity_boost");
    validateVoiceSetting(request.voice_settings.style, "style");
}

std::string buildTextToSpeechJsonBody(const ElevenLabsTextToSpeechRequest& request)
{
    std::ostringstream stream;
    stream << std::boolalpha
           << "{"
           << "\"text\":\"" << escapeJsonString(request.text) << "\","
           << "\"model_id\":\"" << escapeJsonString(request.model_id) << "\","
           << "\"output_format\":\"" << escapeJsonString(request.output_format) << "\","
           << "\"voice_settings\":{"
           << "\"stability\":" << request.voice_settings.stability << ","
           << "\"similarity_boost\":" << request.voice_settings.similarity_boost << ","
           << "\"style\":" << request.voice_settings.style << ","
           << "\"use_speaker_boost\":" << request.voice_settings.use_speaker_boost
           << "}"
           << "}";

    return stream.str();
}
}

ElevenLabsClient::ElevenLabsClient(std::string api_key)
    : ElevenLabsClient(std::move(api_key), kDefaultBaseUrl)
{
}

ElevenLabsClient::ElevenLabsClient(std::string api_key, std::string base_url)
    : api_key_(std::move(api_key))
    , base_url_(std::move(base_url))
{
    if (api_key_.empty()) {
        throw std::invalid_argument("ElevenLabs API key cannot be empty.");
    }

    if (base_url_.empty()) {
        throw std::invalid_argument("ElevenLabs base URL cannot be empty.");
    }
}

ElevenLabsResponse ElevenLabsClient::getModels() const
{
    return performGetRequest(kGetModelsPath, "ElevenLabs models request failed");
}

ElevenLabsResponse ElevenLabsClient::getVoices(int page_size) const
{
    if (page_size < kMinimumVoicePageSize || page_size > kMaximumVoicePageSize) {
        throw std::invalid_argument("ElevenLabs voice page size must be between 1 and 100.");
    }

    return performGetRequest(
        std::string(kGetVoicesPath) + std::to_string(page_size),
        "ElevenLabs voices request failed"
    );
}

ElevenLabsResponse ElevenLabsClient::textToSpeech(const ElevenLabsTextToSpeechRequest& request) const
{
    validateTextToSpeechRequest(request);

    return performPostJsonRequest(
        std::string(kTextToSpeechPathPrefix) + request.voice_id,
        buildTextToSpeechJsonBody(request),
        "ElevenLabs text-to-speech request failed"
    );
}

std::string ElevenLabsClient::buildUrl(const std::string& path) const
{
    if (path.empty() || path.front() != '/') {
        throw std::invalid_argument("ElevenLabs API path must start with '/'.");
    }

    if (base_url_.back() == '/') {
        return base_url_.substr(0, base_url_.size() - 1) + path;
    }

    return base_url_ + path;
}

ElevenLabsResponse ElevenLabsClient::performGetRequest(
    const std::string& path,
    const std::string& error_context
) const
{
    ensureCurlGlobalInitialized();

    CurlHandle curl(curl_easy_init(), curl_easy_cleanup);
    if (curl == nullptr) {
        throw std::runtime_error("Cannot create curl handle for ElevenLabs request.");
    }

    const std::string url = buildUrl(path);
    std::string response_body;

    CurlHeaders headers(nullptr, curl_slist_free_all);
    headers = appendHeader(std::move(headers), kJsonContentTypeHeader);
    headers = appendHeader(std::move(headers), std::string(kApiKeyHeaderPrefix) + api_key_);

    setCurlOption(curl.get(), CURLOPT_CUSTOMREQUEST, "GET");
    setCurlOption(curl.get(), CURLOPT_URL, url.c_str());
    setCurlOption(curl.get(), CURLOPT_FOLLOWLOCATION, kFollowRedirects);
    setCurlOption(curl.get(), CURLOPT_DEFAULT_PROTOCOL, kDefaultProtocol);

    CURLcode code = curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure ElevenLabs request headers: ") + curl_easy_strerror(code)
        );
    }

    code = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeResponseCallback);
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure ElevenLabs response callback: ") + curl_easy_strerror(code)
        );
    }

    code = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response_body);
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure ElevenLabs response buffer: ") + curl_easy_strerror(code)
        );
    }

    code = curl_easy_perform(curl.get());
    if (code != CURLE_OK) {
        throw std::runtime_error(
            error_context + ": " + curl_easy_strerror(code)
        );
    }

    long status_code = 0;
    code = curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status_code);
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot read ElevenLabs response status: ") + curl_easy_strerror(code)
        );
    }

    return ElevenLabsResponse{
        status_code,
        std::move(response_body),
    };
}

ElevenLabsResponse ElevenLabsClient::performPostJsonRequest(
    const std::string& path,
    const std::string& json_body,
    const std::string& error_context
) const
{
    if (json_body.size() > static_cast<size_t>(std::numeric_limits<long>::max())) {
        throw std::invalid_argument("ElevenLabs JSON request body is too large.");
    }

    ensureCurlGlobalInitialized();

    CurlHandle curl(curl_easy_init(), curl_easy_cleanup);
    if (curl == nullptr) {
        throw std::runtime_error("Cannot create curl handle for ElevenLabs request.");
    }

    const std::string url = buildUrl(path);
    std::string response_body;

    CurlHeaders headers(nullptr, curl_slist_free_all);
    headers = appendHeader(std::move(headers), kJsonContentTypeHeader);
    headers = appendHeader(std::move(headers), std::string(kApiKeyHeaderPrefix) + api_key_);

    setCurlOption(curl.get(), CURLOPT_CUSTOMREQUEST, "POST");
    setCurlOption(curl.get(), CURLOPT_URL, url.c_str());
    setCurlOption(curl.get(), CURLOPT_POSTFIELDS, json_body.c_str());
    setCurlOption(curl.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(json_body.size()));
    setCurlOption(curl.get(), CURLOPT_FOLLOWLOCATION, kFollowRedirects);
    setCurlOption(curl.get(), CURLOPT_DEFAULT_PROTOCOL, kDefaultProtocol);

    CURLcode code = curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure ElevenLabs request headers: ") + curl_easy_strerror(code)
        );
    }

    code = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeResponseCallback);
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure ElevenLabs response callback: ") + curl_easy_strerror(code)
        );
    }

    code = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response_body);
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure ElevenLabs response buffer: ") + curl_easy_strerror(code)
        );
    }

    code = curl_easy_perform(curl.get());
    if (code != CURLE_OK) {
        throw std::runtime_error(
            error_context + ": " + curl_easy_strerror(code)
        );
    }

    long status_code = 0;
    code = curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status_code);
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot read ElevenLabs response status: ") + curl_easy_strerror(code)
        );
    }

    return ElevenLabsResponse{
        status_code,
        std::move(response_body),
    };
}

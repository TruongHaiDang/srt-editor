#include "elevenlabs.h"

#include <curl/curl.h>

#include <memory>
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
constexpr int kMinimumVoicePageSize = 1;
constexpr int kMaximumVoicePageSize = 100;

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

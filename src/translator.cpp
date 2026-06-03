#include "translator.h"

namespace
{
constexpr long kFollowRedirects = 1L;
constexpr const char* kDefaultProtocol = "https";
constexpr const char* kJsonContentTypeHeader = "Content-Type: application/json";
constexpr const char* kAuthorizationHeaderPrefix = "Authorization: Bearer ";
constexpr const char* kResponsesPath = "/v1/responses";
constexpr const char* kPromptId = "pmpt_6a0f01bb349481979917ee0c1d43fe4b0952c8d49f597498";
constexpr const char* kPromptVersion = "3";

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

void validateTranslationRequest(const OpenAITranslatorRequest& request)
{
    if (request.source_language.empty()) {
        throw std::invalid_argument("OpenAI translation source language cannot be empty.");
    }

    if (request.target_language.empty()) {
        throw std::invalid_argument("OpenAI translation target language cannot be empty.");
    }

    if (request.content_type.empty()) {
        throw std::invalid_argument("OpenAI translation content type cannot be empty.");
    }

    if (request.content.empty()) {
        throw std::invalid_argument("OpenAI translation content cannot be empty.");
    }
}

std::string buildTranslationJsonBody(const OpenAITranslatorRequest& request)
{
    std::ostringstream stream;
    stream << "{"
           << "\"prompt\":{"
           << "\"id\":\"" << kPromptId << "\","
           << "\"version\":\"" << kPromptVersion << "\","
           << "\"variables\":{"
           << "\"source_language\":\"" << escapeJsonString(request.source_language) << "\","
           << "\"target_language\":\"" << escapeJsonString(request.target_language) << "\","
           << "\"content_type\":\"" << escapeJsonString(request.content_type) << "\","
           << "\"content\":\"" << escapeJsonString(request.content) << "\""
           << "}"
           << "},"
           << "\"input\":[],"
           << "\"text\":{"
           << "\"format\":{"
           << "\"type\":\"json_schema\","
           << "\"name\":\"translated_text\","
           << "\"strict\":true,"
           << "\"schema\":{"
           << "\"type\":\"object\","
           << "\"properties\":{"
           << "\"translated_text\":{"
           << "\"type\":\"string\","
           << "\"description\":\"The translated text result\""
           << "}"
           << "},"
           << "\"required\":[\"translated_text\"],"
           << "\"additionalProperties\":false"
           << "}"
           << "},"
           << "\"verbosity\":\"medium\""
           << "},"
           << "\"reasoning\":{"
           << "\"summary\":\"auto\""
           << "},"
           << "\"store\":true,"
           << "\"include\":["
           << "\"reasoning.encrypted_content\","
           << "\"web_search_call.action.sources\""
           << "]"
           << "}";

    return stream.str();
}
}

OpenAITranslatorClient::OpenAITranslatorClient(std::string api_key)
    : OpenAITranslatorClient(std::move(api_key), kDefaultBaseUrl)
{
}

OpenAITranslatorClient::OpenAITranslatorClient(std::string api_key, std::string base_url)
    : api_key_(std::move(api_key))
    , base_url_(std::move(base_url))
{
    if (api_key_.empty()) {
        throw std::invalid_argument("OpenAI API key cannot be empty.");
    }

    if (base_url_.empty()) {
        throw std::invalid_argument("OpenAI base URL cannot be empty.");
    }
}

OpenAITranslatorResponse OpenAITranslatorClient::translate(
    const OpenAITranslatorRequest& request
) const
{
    validateTranslationRequest(request);

    return performPostJsonRequest(
        kResponsesPath,
        buildTranslationJsonBody(request),
        "OpenAI translation request failed"
    );
}

std::string OpenAITranslatorClient::buildUrl(const std::string& path) const
{
    if (path.empty() || path.front() != '/') {
        throw std::invalid_argument("OpenAI API path must start with '/'.");
    }

    if (base_url_.back() == '/') {
        return base_url_.substr(0, base_url_.size() - 1) + path;
    }

    return base_url_ + path;
}

OpenAITranslatorResponse OpenAITranslatorClient::performPostJsonRequest(
    const std::string& path,
    const std::string& json_body,
    const std::string& error_context
) const
{
    if (json_body.size() > static_cast<size_t>(std::numeric_limits<long>::max())) {
        throw std::invalid_argument("OpenAI JSON request body is too large.");
    }

    ensureCurlGlobalInitialized();

    CurlHandle curl(curl_easy_init(), curl_easy_cleanup);
    if (curl == nullptr) {
        throw std::runtime_error("Cannot create curl handle for OpenAI request.");
    }

    const std::string url = buildUrl(path);
    std::string response_body;

    CurlHeaders headers(nullptr, curl_slist_free_all);
    headers = appendHeader(std::move(headers), kJsonContentTypeHeader);
    headers = appendHeader(std::move(headers), std::string(kAuthorizationHeaderPrefix) + api_key_);

    setCurlOption(curl.get(), CURLOPT_CUSTOMREQUEST, "POST");
    setCurlOption(curl.get(), CURLOPT_URL, url.c_str());
    setCurlOption(curl.get(), CURLOPT_POSTFIELDS, json_body.c_str());
    setCurlOption(curl.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(json_body.size()));
    setCurlOption(curl.get(), CURLOPT_FOLLOWLOCATION, kFollowRedirects);
    setCurlOption(curl.get(), CURLOPT_DEFAULT_PROTOCOL, kDefaultProtocol);

    CURLcode code = curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure OpenAI request headers: ") + curl_easy_strerror(code)
        );
    }

    code = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeResponseCallback);
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure OpenAI response callback: ") + curl_easy_strerror(code)
        );
    }

    code = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response_body);
    if (code != CURLE_OK) {
        throw std::runtime_error(
            std::string("Cannot configure OpenAI response buffer: ") + curl_easy_strerror(code)
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
            std::string("Cannot read OpenAI response status: ") + curl_easy_strerror(code)
        );
    }

    return OpenAITranslatorResponse{
        status_code,
        std::move(response_body),
    };
}

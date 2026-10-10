#include "openai.h"

#include <curl/curl.h>

#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace
{
constexpr long kFollowRedirects = 0L;
constexpr const char* kDefaultProtocol = "https";
constexpr const char* kJsonContentTypeHeader = "Content-Type: application/json";
constexpr const char* kAuthorizationHeaderPrefix = "Authorization: Bearer ";
constexpr const char* kSpeechPath = "/v1/audio/speech";

size_t writeResponseCallback(char* contents, size_t size, size_t nmemb, void* user_data)
{
    if (size != 0 && nmemb > std::numeric_limits<size_t>::max() / size) {
        return 0;
    }
    const size_t byte_count = size * nmemb;
    auto* response_body = static_cast<std::string*>(user_data);
    try {
        response_body->append(contents, byte_count);
    } catch (...) {
        return 0;
    }
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

void validateTextToSpeechRequest(const OpenAITextToSpeechRequest& request)
{
    if (request.text.empty() || request.model.empty() || request.voice.empty()) {
        throw std::invalid_argument("OpenAI speech text, model and voice cannot be empty.");
    }
    if (!std::isfinite(request.speed) || request.speed < 0.25 || request.speed > 4.0) {
        throw std::invalid_argument("OpenAI speech speed must be between 0.25 and 4.0.");
    }
    if (request.response_format != "mp3" && request.response_format != "opus" &&
        request.response_format != "aac" && request.response_format != "flac" &&
        request.response_format != "wav" && request.response_format != "pcm") {
        throw std::invalid_argument("Unsupported OpenAI speech response format.");
    }
    if (!request.instructions.empty() && (request.model == "tts-1" || request.model == "tts-1-hd")) {
        throw std::invalid_argument("OpenAI tts-1 and tts-1-hd do not support instructions.");
    }
}

std::string buildTextToSpeechJsonBody(const OpenAITextToSpeechRequest& request)
{
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setprecision(17)
           << "{\"input\":\"" << escapeJsonString(request.text)
           << "\",\"model\":\"" << escapeJsonString(request.model)
           << "\",\"voice\":\"" << escapeJsonString(request.voice)
           << "\",\"response_format\":\"" << escapeJsonString(request.response_format)
           << "\",\"speed\":" << request.speed;
    if (!request.instructions.empty()) {
        stream << ",\"instructions\":\"" << escapeJsonString(request.instructions) << "\"";
    }
    stream << "}";
    return stream.str();
}
}

OpenAIClient::OpenAIClient(std::string api_key)
    : OpenAIClient(std::move(api_key), kDefaultBaseUrl)
{
}

OpenAIClient::OpenAIClient(std::string api_key, std::string base_url)
    : api_key_(std::move(api_key))
    , base_url_(std::move(base_url))
{
    if (api_key_.empty() || api_key_.find_first_of("\r\n") != std::string::npos ||
        api_key_.find('\0') != std::string::npos) {
        throw std::invalid_argument("OpenAI API key cannot be empty or contain header control characters.");
    }

    if (base_url_.empty()) {
        throw std::invalid_argument("OpenAI base URL cannot be empty.");
    }
}

OpenAIResponse OpenAIClient::textToSpeech(
    const OpenAITextToSpeechRequest& request
) const
{
    validateTextToSpeechRequest(request);

    return performPostJsonRequest(
        kSpeechPath,
        buildTextToSpeechJsonBody(request),
        "OpenAI text-to-speech request failed"
    );
}

std::string OpenAIClient::buildUrl(const std::string& path) const
{
    if (path.empty() || path.front() != '/') {
        throw std::invalid_argument("OpenAI API path must start with '/'.");
    }

    if (base_url_.back() == '/') {
        return base_url_.substr(0, base_url_.size() - 1) + path;
    }

    return base_url_ + path;
}

OpenAIResponse OpenAIClient::performPostJsonRequest(
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

    setCurlOption(curl.get(), CURLOPT_CONNECTTIMEOUT, 30L);
    setCurlOption(curl.get(), CURLOPT_TIMEOUT, 180L);
    setCurlOption(curl.get(), CURLOPT_NOSIGNAL, 1L);
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

    return OpenAIResponse{
        status_code,
        std::move(response_body),
    };
}

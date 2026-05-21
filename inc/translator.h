#pragma once

#include <curl/curl.h>

#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

struct OpenAITranslatorResponse final
{
    long status_code = 0;
    std::string body;
};

struct OpenAITranslatorRequest final
{
    std::string source_language;
    std::string target_language;
    std::string content_type;
    std::string content;
};

/**
 * Minimal OpenAI Responses API client for the translation prompt.
 *
 * Input: API key supplied by the caller, plus prompt variables required by the translation prompt.
 * Output: HTTP status code and raw JSON response body from /v1/responses.
 * Errors: throws std::invalid_argument for invalid input and std::runtime_error for curl/runtime failures.
 * Example:
 *   OpenAITranslatorClient client(api_key);
 *   OpenAITranslatorRequest request{"English", "Vietnamese", "subtitle", "Hello world."};
 *   OpenAITranslatorResponse response = client.translate(request);
 */
class OpenAITranslatorClient final
{
public:
    explicit OpenAITranslatorClient(std::string api_key);
    OpenAITranslatorClient(std::string api_key, std::string base_url);

    OpenAITranslatorResponse translate(const OpenAITranslatorRequest& request) const;

private:
    static constexpr const char* kDefaultBaseUrl = "https://api.openai.com";

    std::string buildUrl(const std::string& path) const;
    OpenAITranslatorResponse performPostJsonRequest(
        const std::string& path,
        const std::string& json_body,
        const std::string& error_context
    ) const;

    std::string api_key_;
    std::string base_url_;
};

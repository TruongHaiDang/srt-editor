#pragma once

#include <string>

struct OpenAIResponse final
{
    long status_code = 0;
    std::string body;
};

struct OpenAITextToSpeechRequest final
{
    std::string text;
    std::string model = "gpt-4o-mini-tts";
    std::string voice = "coral";
    std::string response_format = "mp3";
    double speed = 1.0;
    std::string instructions;
};

/**
 * Synchronous OpenAI speech client backed by libcurl.
 * Pass the API key from settings or OPENAI_API_KEY to the constructor.
 * On success, body contains binary audio (including possible NUL bytes).
 * HTTP errors return their status and raw error body; check status before saving audio.
 * Invalid input throws std::invalid_argument; transport failures throw std::runtime_error.
 * Calls can block up to 180 seconds; use a worker thread in UI code.
 * End users must be told that the generated voice is AI-generated.
 */
class OpenAIClient final
{
public:
    explicit OpenAIClient(std::string api_key);
    OpenAIClient(std::string api_key, std::string base_url);

    OpenAIResponse textToSpeech(const OpenAITextToSpeechRequest& request) const;

private:
    static constexpr const char* kDefaultBaseUrl = "https://api.openai.com";

    std::string buildUrl(const std::string& path) const;
    OpenAIResponse performPostJsonRequest(
        const std::string& path,
        const std::string& json_body,
        const std::string& error_context
    ) const;

    std::string api_key_;
    std::string base_url_;
};

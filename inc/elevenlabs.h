#pragma once

#include <string>

struct ElevenLabsResponse final
{
    long status_code = 0;
    std::string body;
};

/**
 * Minimal ElevenLabs API client backed by libcurl.
 *
 * Input: API key supplied by the caller, typically from app settings or an environment variable.
 * Output: typed response objects containing the HTTP status code and raw JSON body.
 * Errors: throws std::invalid_argument for invalid configuration and std::runtime_error for curl/runtime failures.
 * Example:
 *   ElevenLabsClient client(api_key);
 *   ElevenLabsResponse response = client.getModels();
 *   ElevenLabsResponse voices = client.getVoices();
 */
class ElevenLabsClient final
{
public:
    explicit ElevenLabsClient(std::string api_key);
    ElevenLabsClient(std::string api_key, std::string base_url);

    ElevenLabsResponse getModels() const;
    ElevenLabsResponse getVoices(int page_size = 100) const;

private:
    static constexpr const char* kDefaultBaseUrl = "https://api.elevenlabs.io";

    std::string buildUrl(const std::string& path) const;
    ElevenLabsResponse performGetRequest(const std::string& path, const std::string& error_context) const;

    std::string api_key_;
    std::string base_url_;
};

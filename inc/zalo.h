#pragma once

#include <string>

struct ZaloResponse final
{
    long status_code = 0;
    std::string body;
};

enum class ZaloAudioEncoding
{
    Wav = 0,
    Mp3 = 1,
};

struct ZaloTextToSpeechRequest final
{
    std::string text;
    int speaker_id = 1;
    double speed = 1.0;
    ZaloAudioEncoding encoding = ZaloAudioEncoding::Wav;
};

/**
 * Minimal Zalo AI text-to-speech client backed by libcurl.
 *
 * synthesize() returns Zalo's JSON response. On success, the generated audio
 * URL is stored in data.url; use downloadAudio() to retrieve its bytes.
 * HTTP error responses are returned to the caller and transport/configuration
 * errors throw std::runtime_error or std::invalid_argument.
 */
class ZaloClient final
{
public:
    explicit ZaloClient(std::string api_key);
    ZaloClient(std::string api_key, std::string base_url);

    ZaloResponse synthesize(const ZaloTextToSpeechRequest& request) const;
    ZaloResponse downloadAudio(
        const std::string& audio_url,
        int max_attempts = 4,
        int retry_delay_ms = 750
    ) const;

private:
    static constexpr const char* kDefaultBaseUrl = "https://api.zalo.ai";

    std::string buildUrl(const std::string& path) const;

    std::string api_key_;
    std::string base_url_;
};

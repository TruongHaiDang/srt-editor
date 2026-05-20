#pragma once

#include <string>

struct ElevenLabsResponse final
{
    long status_code = 0;
    std::string body;
};

struct ElevenLabsVoiceSettings final
{
    double stability = 0.5;
    double similarity_boost = 0.75;
    double style = 0.0;
    bool use_speaker_boost = true;
};

struct ElevenLabsTextToSpeechRequest final
{
    std::string voice_id = "21m00Tcm4TlvDq8ikWAM";
    std::string text;
    std::string model_id = "eleven_multilingual_v2";
    std::string output_format = "mp3_44100_128";
    ElevenLabsVoiceSettings voice_settings;
};

/**
 * Minimal ElevenLabs API client backed by libcurl.
 *
 * Input: API key supplied by the caller, typically from app settings or an environment variable.
 * Output: typed response objects containing the HTTP status code and raw response body.
 *         Text-to-speech audio bytes are returned in ElevenLabsResponse::body.
 * Errors: throws std::invalid_argument for invalid configuration and std::runtime_error for curl/runtime failures.
 * Example:
 *   ElevenLabsClient client(api_key);
 *   ElevenLabsResponse response = client.getModels();
 *   ElevenLabsResponse voices = client.getVoices();
 *   ElevenLabsTextToSpeechRequest request;
 *   request.text = "Hello world.";
 *   ElevenLabsResponse audio = client.textToSpeech(request);
 */
class ElevenLabsClient final
{
public:
    explicit ElevenLabsClient(std::string api_key);
    ElevenLabsClient(std::string api_key, std::string base_url);

    ElevenLabsResponse getModels() const;
    ElevenLabsResponse getVoices(int page_size = 100) const;
    ElevenLabsResponse textToSpeech(const ElevenLabsTextToSpeechRequest& request) const;

private:
    static constexpr const char* kDefaultBaseUrl = "https://api.elevenlabs.io";

    std::string buildUrl(const std::string& path) const;
    ElevenLabsResponse performGetRequest(const std::string& path, const std::string& error_context) const;
    ElevenLabsResponse performPostJsonRequest(
        const std::string& path,
        const std::string& json_body,
        const std::string& error_context
    ) const;

    std::string api_key_;
    std::string base_url_;
};

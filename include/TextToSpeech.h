#pragma once

#ifndef __TEXT_TO_SPEECH_H__
#define __TEXT_TO_SPEECH_H__

#include <string>
#include <curl/curl.h>

class TextToSpeech
{
private:
    std::string openaiApiKey;
    std::string openaiInstructions;
    std::string ttsOpenaiUrl = "https://api.openai.com/v1/audio/speech";
    std::string openaiVoices[10] = {
        "alloy",
        "ash",
        "ballad",
        "coral",
        "echo",
        "fable",
        "nova",
        "onyx",
        "sage",
        "shimmer"
    };
    std::string openaiOutputFormat[6] = {
        "MP3",
        "Opus",
        "AAC",
        "FLAC",
        "WAV",
        "PCM"
    };
    std::string openaiModels[3] = {
        "tts-1",
        "tts-1-hd",
        "gpt-4o-mini-tts"
    };

public:
    TextToSpeech();
    ~TextToSpeech();

    void openaiTextToSpeech(std::string text, std::string outputDir, float speed);
    void elevenlabsTextToSpeech(std::string text, std::string outputDir);
};

#endif

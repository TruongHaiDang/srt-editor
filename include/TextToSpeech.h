#pragma once

#ifndef __TEXT_TO_SPEECH_H__
#define __TEXT_TO_SPEECH_H__

#include <string>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>

class TextToSpeech
{
private:
    std::string openaiApiKey;
    std::string elevenlabsApiKey;
    std::string openaiInstructions;
    
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
    std::string openaiModels[3] = {
        "tts-1",
        "tts-1-hd",
        "gpt-4o-mini-tts"
    };

public:
    TextToSpeech();
    ~TextToSpeech();

    void setOpenaiApiKey(const std::string& apiKey);
    std::string getOpenaiApiKey() const;
    void setElevenlabsApiKey(const std::string& apiKey);
    std::string getElevenlabsApiKey() const;

    void openaiTextToSpeech(std::string text, std::string outputDir, float speed, std::string model, std::string voice, std::string instructions, std::string outputFile = "");
    void elevenlabsTextToSpeech(std::string text, std::string outputDir, std::string voiceId, std::string modelId, std::string outputFormat, std::string outputFile = "");
};

#endif

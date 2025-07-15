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

public:
    TextToSpeech();
    ~TextToSpeech();

    void openaiTextToSpeech(std::string text, std::string outputDir, std::string model, std::string voice, std::string instructions, std::string outputFile, std::string apiKey);
    void elevenlabsTextToSpeech(std::string text, std::string outputDir, std::string voiceId, std::string modelId, std::string outputFormat, std::string outputFile, std::string apiKey);
};

#endif

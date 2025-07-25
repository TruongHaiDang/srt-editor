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

extern "C"
{
#include <libavformat/avformat.h>
}

class TextToSpeech
{
private:
public:
    TextToSpeech();
    ~TextToSpeech();

    struct AudioTime {
        int hours;
        int minutes;
        int seconds;
        int milliseconds;
    };
    

    std::string openaiTextToSpeech(std::string text, std::string outputDir, std::string model, std::string voice, std::string instructions, std::string outputFile, std::string apiKey);
    std::string elevenlabsTextToSpeech(std::string text, std::string outputDir, std::string voiceId, std::string modelId, std::string outputFormat, std::string outputFile, std::string apiKey);
    AudioTime getAudioLength(const std::string& filePath);
};

#endif

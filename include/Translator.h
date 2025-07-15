#pragma once

#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

#include <curl/curl.h>
#include <string>
#include <nlohmann/json.hpp>

class Translator
{
private:
    static size_t writeCallback(void* ptr, size_t size, size_t nmemb, void* userData);

public:
    Translator();
    ~Translator();

    std::string openaiTranslate(std::string subtitle, std::string srcLang, std::string dstLang, std::string apiKey, float temperature = 1.0f, float topp = 1.0f, int maxTokens = 1024);
    std::string githubModelsTranslate(std::string subtitle, std::string srcLang, std::string dstLang, std::string token, float temperature = 1.0f, float topp = 1.0f, int maxTokens = 1024);
};

#endif

#pragma once

#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

#include <curl/curl.h>
#include <string>
#include <nlohmann/json.hpp>

class Translator
{
private:
    std::string githubToken;
    std::string openaiApiKey;
    static size_t writeCallback(void* ptr, size_t size, size_t nmemb, void* userData);

public:
    Translator();
    ~Translator();

    void setGithubToken(const std::string& token);
    std::string getGithubToken() const;
    void setOpenaiApiKey(const std::string& apiKey);
    std::string getOpenaiApiKey() const;

    std::string openaiTranslate(std::string subtitle, std::string lang, std::string modelName);
    std::string githubModelsTranslate(std::string subtitle, std::string lang, std::string modelName);
};

#endif

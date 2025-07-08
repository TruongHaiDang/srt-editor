#include "Translator.h"

Translator::Translator()
{
}

Translator::~Translator()
{
}

size_t Translator::writeCallback(void* ptr, size_t size, size_t nmemb, void* userData) {
    size_t total = size * nmemb;
    static_cast<std::string*>(userData)->append((char*)ptr, total);
    return total;
}

std::string Translator::openaiTranslate(std::string subtitle, std::string lang, std::string modelName)
{
    // Tạo prompt dịch
    std::string prompt = "Dịch đoạn sau sang " + lang + ": " + subtitle;

    // Tạo JSON body
    nlohmann::json bodyJson = {
        {"model", modelName},
        {"messages", {
            { { "role", "system" }, { "content", "You are a professional translator. Translate the following text as accurately and naturally as possible." } },
            { { "role", "user" }, { "content", prompt } }
        }}
    };
    std::string body = bodyJson.dump();

    CURL *curl = curl_easy_init();
    if (!curl) return "";

    struct curl_slist *headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + openaiApiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Translator::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);

    std::string result;
    if (res == CURLE_OK) {
        // Phân tích response JSON để lấy nội dung dịch
        try {
            auto jsonRes = nlohmann::json::parse(response);
            result = jsonRes["choices"][0]["message"]["content"].get<std::string>();
        } catch (...) {
            result = "";
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return result;
}

std::string Translator::githubModelsTranslate(std::string subtitle, std::string lang, std::string modelName)
{
    // Tạo prompt dịch
    std::string prompt = "Dịch đoạn sau sang " + lang + ": " + subtitle;

    // Tạo JSON body
    nlohmann::json bodyJson = {
        {"messages", {
            { { "role", "system" }, { "content", "You are a professional translator. Translate the following text as accurately and naturally as possible." } },
            { { "role", "user" }, { "content", prompt } }
        }},
        {"temperature", 1.0},
        {"top_p", 1.0},
        {"model", modelName}
    };
    std::string body = bodyJson.dump();

    CURL *curl = curl_easy_init();
    if (!curl) return "";

    struct curl_slist *headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + githubToken;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, "https://models.github.ai/inference/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Translator::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);

    std::string result;
    if (res == CURLE_OK) {
        // Phân tích response JSON để lấy nội dung dịch
        try {
            auto jsonRes = nlohmann::json::parse(response);
            result = jsonRes["choices"][0]["message"]["content"].get<std::string>();
        } catch (...) {
            result = "";
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return result;
}

void Translator::setGithubToken(const std::string& token) {
    githubToken = token;
}

std::string Translator::getGithubToken() const {
    return githubToken;
}

void Translator::setOpenaiApiKey(const std::string& apiKey) {
    openaiApiKey = apiKey;
}

std::string Translator::getOpenaiApiKey() const {
    return openaiApiKey;
}

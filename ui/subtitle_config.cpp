#include "subtitle_config.h"

SubtitleConfig::SubtitleConfig(QWidget* parent): QDialog(parent), ui(new Ui::SubtitleConfig)
{
    ui->setupUi(this);
}

SubtitleConfig::~SubtitleConfig()
{
}

size_t SubtitleConfig::writeCallback(void *ptr, size_t size, size_t nmemb, void *userData)
{
    size_t total = size * nmemb;
    static_cast<std::string*>(userData)->append((char*)ptr, total);
    return total;
}

void SubtitleConfig::getOpenaiChatModels()
{
    CURL *curl = curl_easy_init();
    if (!curl) return;

    struct curl_slist *headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + openaiApiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/models");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SubtitleConfig::writeCallback);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK)
    {
        auto jsonRes = nlohmann::json::parse(response);
        QString jsonStr = QString::fromStdString(jsonRes.dump(4));
        qDebug().noquote() << jsonStr;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

#include "subtitle_config.h"

SubtitleConfig::SubtitleConfig(QWidget *parent) : QDialog(parent), ui(new Ui::SubtitleConfig)
{
    ui->setupUi(this);

    QSettings settings("haidanghth910", "srteditor");
    std::string translateProvider = settings.value("translate/provider", "OpenAI").toString().toStdString();
    if (translateProvider == "OpenAI")
    {
        this->getOpenaiModels();
    }
    else if (translateProvider == "Github Models")
    {
        this->getGithubChatModels();
    };
    
    std::string speechProvider = settings.value("tts/provider", "OpenAI").toString().toStdString();
    if (speechProvider == "OpenAI")
    {
        ui->speechVoices->addItems(this->openaiSpeechVoices);
        ui->speechModels->addItems(this->openaiSpeechModels);
    }
    else if (speechProvider == "ElevenLabs")
    {
        this->getElevenlabsSpeechModels();
        this->getElevenlabsSpeechVoices();
    }
}

SubtitleConfig::~SubtitleConfig()
{
}

size_t SubtitleConfig::writeCallback(void *ptr, size_t size, size_t nmemb, void *userData)
{
    size_t total = size * nmemb;
    static_cast<std::string *>(userData)->append((char *)ptr, total);
    return total;
}

void SubtitleConfig::getOpenaiModels()
{
    QSettings settings("haidanghth910", "srteditor");
    this->openaiApiKey = settings.value("translate/apiKey", "").toString().toStdString();

    CURL *curl = curl_easy_init();
    if (curl)
    {
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
            if (jsonRes.contains("data") && jsonRes["data"].is_array())
            {
                this->openaiChatModels.clear();
                this->openaiSpeechModels.clear();
                for (const auto &model : jsonRes["data"])
                {
                    if (model.contains("id"))
                    {
                        std::string mod = model["id"].get<std::string>();
                        if (mod.find("gpt") != std::string::npos)
                            this->openaiChatModels.append(QString::fromStdString(mod));
                        if (mod.find("tts") != std::string::npos || mod.find("whisper") != std::string::npos)
                            this->openaiSpeechModels.append(QString::fromStdString(mod));
                    }
                };

                ui->translateModels->addItems(this->openaiChatModels);
            }
        }

        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}

void SubtitleConfig::getElevenlabsSpeechModels()
{
    QSettings settings("haidanghth910", "srteditor");
    this->elevenlabsApiKey = settings.value("tts/apiKey", "").toString().toStdString();

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.elevenlabs.io/v1/models");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = nullptr;
        std::string authHeader = "xi-api-key: " + this->elevenlabsApiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SubtitleConfig::writeCallback);
        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            // Parse JSON trả về
            nlohmann::json jsonRes = nlohmann::json::parse(response);
            // Xóa danh sách cũ (nếu có)
            this->elevenlabsSpeechModels.clear();
            // Duyệt từng phần tử trong mảng JSON
            if (jsonRes.is_array())
            {
                for (const nlohmann::json &model : jsonRes)
                {
                    // Ghi log model ra debug
                    if (model.contains("model_id") && model["model_id"].is_string())
                    {
                        QString modelId = QString::fromStdString(model["model_id"].get<std::string>());
                        this->elevenlabsSpeechModels.append(modelId);
                    }
                }
            }
            // Nếu muốn hiển thị lên combobox:
            ui->speechModels->addItems(this->elevenlabsSpeechModels);
        }
        else
        {
            qDebug() << "Curl failed:" << curl_easy_strerror(res);  // In ra thông điệp lỗi
            qDebug() << "Error code:" << res;                       // In ra số lỗi, ví dụ 28
        };
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}

void SubtitleConfig::getElevenlabsSpeechVoices()
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.elevenlabs.io/v1/voices");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = nullptr;
        std::string authHeader = "xi-api-key: " + this->elevenlabsApiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SubtitleConfig::writeCallback);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            nlohmann::json jsonRes = nlohmann::json::parse(response);
            this->elevenlabsSpeechVoices.clear();
            if (jsonRes.contains("voices") && jsonRes["voices"].is_array())
            {
                for (const auto &voice : jsonRes["voices"])
                {
                    if (voice.contains("voice_id") && voice["voice_id"].is_string())
                    {
                        QString voiceId = QString::fromStdString(voice["voice_id"].get<std::string>());
                        this->elevenlabsSpeechVoices.append(voiceId);
                    }
                }
            }
            // Hiển thị lên combobox:
            ui->speechVoices->addItems(this->elevenlabsSpeechVoices);
        }
        else
        {
            qDebug() << "Curl failed:" << curl_easy_strerror(res);  // In ra thông điệp lỗi
            qDebug() << "Error code:" << res;                       // In ra số lỗi, ví dụ 28
        };
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}

void SubtitleConfig::getGithubChatModels()
{

}

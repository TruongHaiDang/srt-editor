#include "subtitle_config.h"

SubtitleConfig::SubtitleConfig(QWidget *parent) : QDialog(parent), ui(new Ui::SubtitleConfig)
{
    ui->setupUi(this);
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

void SubtitleConfig::getOpenaiChatModels()
{
    QSettings settings("haidanghth910", "srteditor");

    // Check date before updating
    const qint64 todayDay = QDate::currentDate().toJulianDay();
    const qint64 cachedDay = settings.value("cache/getOpenaiChatModels/day", 0).toLongLong();
    const bool needRefresh = (cachedDay != todayDay);
    
    // Nếu không cần refresh, load từ cache và return
    if (!needRefresh) {
        this->openaiChatModels = settings.value("cache/getOpenaiChatModels/data").toStringList();
        return;
    }
    
    // Cập nhật ngày cache
    settings.setValue("cache/getOpenaiChatModels/day", todayDay);

    this->openaiApiKey = settings.value("translate/apiKey", "").toString().toStdString();
    if (this->openaiApiKey.empty())
        return;

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
            if (jsonRes.contains("data") && jsonRes["data"].is_array())
            {
                this->openaiChatModels.clear();
                for (const auto &model : jsonRes["data"])
                {
                    if (model.contains("id"))
                    {
                        std::string mod = model["id"].get<std::string>();
                        if (mod.find("gpt") != std::string::npos)
                            this->openaiChatModels.append(QString::fromStdString(mod));
                    }
                };
                // Cache dữ liệu
                settings.setValue("cache/getOpenaiChatModels/data", this->openaiChatModels);
                ui->translateModels->addItems(this->openaiChatModels);
            }
        }

        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}

void SubtitleConfig::getOpenaiSpeechModels()
{
    QSettings settings("haidanghth910", "srteditor");

    // Check date before updating
    const qint64 todayDay = QDate::currentDate().toJulianDay();
    const qint64 cachedDay = settings.value("cache/getOpenaiSpeechModels/day", 0).toLongLong();
    const bool needRefresh = (cachedDay != todayDay);
    
    // Nếu không cần refresh, load từ cache và return
    if (!needRefresh) {
        this->openaiSpeechModels = settings.value("cache/getOpenaiSpeechModels/data").toStringList();
        return;
    }
    
    // Cập nhật ngày cache
    settings.setValue("cache/getOpenaiSpeechModels/day", todayDay);

    this->openaiApiKey = settings.value("tts/apiKey", "").toString().toStdString();
    if (this->openaiApiKey.empty())
        return;

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
            if (jsonRes.contains("data") && jsonRes["data"].is_array())
            {
                this->openaiSpeechModels.clear();
                for (const auto &model : jsonRes["data"])
                {
                    if (model.contains("id"))
                    {
                        std::string mod = model["id"].get<std::string>();
                        if (mod.find("tts") != std::string::npos || mod.find("whisper") != std::string::npos)
                            this->openaiSpeechModels.append(QString::fromStdString(mod));
                    }
                };
                // Cache dữ liệu
                settings.setValue("cache/getOpenaiSpeechModels/data", this->openaiSpeechModels);
                ui->speechVoices->addItems(this->openaiSpeechVoices);
                ui->speechModels->addItems(this->openaiSpeechModels);
            }
        }

        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}

void SubtitleConfig::getElevenlabsSpeechModels()
{
    QSettings settings("haidanghth910", "srteditor");
    
    // Check date before updating
    const qint64 todayDay = QDate::currentDate().toJulianDay();
    const qint64 cachedDay = settings.value("cache/getElevenlabsSpeechModels/day", 0).toLongLong();
    const bool needRefresh = (cachedDay != todayDay);
    
    // Nếu không cần refresh, load từ cache và return
    if (!needRefresh) {
        this->elevenlabsSpeechModels = settings.value("cache/getElevenlabsSpeechModels/data").toStringList();
        return;
    }
    
    // Cập nhật ngày cache
    settings.setValue("cache/getElevenlabsSpeechModels/day", todayDay);
    
    this->elevenlabsApiKey = settings.value("tts/apiKey", "").toString().toStdString();
    if (this->elevenlabsApiKey.empty())
        return;

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
            // Cache dữ liệu và hiển thị lên combobox
            settings.setValue("cache/getElevenlabsSpeechModels/data", this->elevenlabsSpeechModels);
            ui->speechModels->addItems(this->elevenlabsSpeechModels);
        }
        else
        {
            qDebug() << "Curl failed:" << curl_easy_strerror(res); // In ra thông điệp lỗi
            qDebug() << "Error code:" << res;                      // In ra số lỗi, ví dụ 28
        };
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}

void SubtitleConfig::getElevenlabsSpeechVoices()
{
    QSettings settings("haidanghth910", "srteditor");
    
    // Check date before updating
    const qint64 todayDay = QDate::currentDate().toJulianDay();
    const qint64 cachedDay = settings.value("cache/getElevenlabsSpeechVoices/day", 0).toLongLong();
    const bool needRefresh = (cachedDay != todayDay);
    
    // Nếu không cần refresh, load từ cache và return
    if (!needRefresh) {
        QStringList voiceNames = settings.value("cache/getElevenlabsSpeechVoices/data").toStringList();
        // Khôi phục map voiceNameToId từ cache
        for (const QString &name : voiceNames) {
            QString id = settings.value(QString("cache/getElevenlabsSpeechVoices/voiceMap/%1").arg(name)).toString();
            if (!id.isEmpty()) {
                this->elevenlabsVoiceNameToId[name] = id;
            }
        }
        this->elevenlabsSpeechVoices = voiceNames;
        return;
    }
    
    // Cập nhật ngày cache
    settings.setValue("cache/getElevenlabsSpeechVoices/day", todayDay);
    
    this->elevenlabsApiKey = settings.value("tts/apiKey", "").toString().toStdString();
    if (this->elevenlabsApiKey.empty())
        return;

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
            this->elevenlabsVoiceNameToId.clear();
            QStringList voiceNames;
            if (jsonRes.contains("voices") && jsonRes["voices"].is_array())
            {
                for (const auto &voice : jsonRes["voices"])
                {
                    if (voice.contains("name") && voice.contains("voice_id") &&
                        voice["name"].is_string() && voice["voice_id"].is_string())
                    {
                        QString name = QString::fromStdString(voice["name"].get<std::string>());
                        QString id = QString::fromStdString(voice["voice_id"].get<std::string>());
                        this->elevenlabsVoiceNameToId[name] = id;
                        voiceNames.append(name);
                    }
                }
            }
            // Cache dữ liệu
            settings.setValue("cache/getElevenlabsSpeechVoices/data", voiceNames);
            // Cache map voiceNameToId
            for (const QString &name : voiceNames) {
                settings.setValue(QString("cache/getElevenlabsSpeechVoices/voiceMap/%1").arg(name), 
                                this->elevenlabsVoiceNameToId.value(name));
            }
            this->elevenlabsSpeechVoices = voiceNames;
            
            ui->speechVoices->clear();
            ui->speechVoices->addItems(voiceNames);
        }
        else
        {
            qDebug() << "Curl failed:" << curl_easy_strerror(res);
            qDebug() << "Error code:" << res;
        }
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}

void SubtitleConfig::getGithubChatModels()
{
    QSettings settings("haidanghth910", "srteditor");
    
    // Check date before updating
    const qint64 todayDay = QDate::currentDate().toJulianDay();
    const qint64 cachedDay = settings.value("cache/getGithubChatModels/day", 0).toLongLong();
    const bool needRefresh = (cachedDay != todayDay);
    
    // Nếu không cần refresh, load từ cache và return
    if (!needRefresh) {
        this->githubChatModels = settings.value("cache/getGithubChatModels/data").toStringList();
        return;
    }
    
    // Cập nhật ngày cache
    settings.setValue("cache/getGithubChatModels/day", todayDay);
    
    this->githubToken = settings.value("translate/apiKey", "").toString().toStdString();
    if (this->githubToken.empty())
        return;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_URL, "https://models.github.ai/catalog/models");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "X-GitHub-Api-Version: 2022-11-28");
        headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
        std::string authHeader = "Authorization: Bearer " + this->githubToken;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SubtitleConfig::writeCallback);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            nlohmann::json jsonRes = nlohmann::json::parse(response);
            this->githubChatModels.clear();
            if (jsonRes.is_array())
            {
                for (const auto &model : jsonRes)
                {
                    if (model.contains("id") && model["id"].is_string())
                    {
                        QString modelId = QString::fromStdString(model["id"].get<std::string>());
                        this->githubChatModels.append(modelId);
                    }
                }
            }
            // Cache dữ liệu
            settings.setValue("cache/getGithubChatModels/data", this->githubChatModels);
            ui->translateModels->addItems(this->githubChatModels);
        }
        else
        {
            qDebug() << "Curl failed:" << curl_easy_strerror(res);
            qDebug() << "Error code:" << res;
        }
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}

std::map<QString, QVariant> SubtitleConfig::getConfigs()
{
    std::map<QString, QVariant> configs;

    // Speech
    configs["speechModel"] = ui->speechModels->currentText();
    configs["speechVoiceName"] = ui->speechVoices->currentText();
    // Lấy voice_id từ map ánh xạ
    configs["speechVoiceId"] = elevenlabsVoiceNameToId.value(ui->speechVoices->currentText());
    configs["speechSpeed"] = ui->speechSpeed->value();
    configs["speechInstructions"] = ui->openaiSpeechInstructions->toPlainText();
    configs["speechLanguage"] = ui->speechLanguage->currentText();
    configs["speechFileFormat"] = ui->speechFileFormat->currentText();

    // Translate
    configs["translateModel"] = ui->translateModels->currentText();
    configs["srcLang"] = ui->srcLanguage->currentText();
    configs["dstLang"] = ui->dstLanguage->currentText();

    return configs;
}

/**
 * Lưu config cho một subtitle item vào QSettings group.
 * @param itemId: Định danh subtitle item (ví dụ: số thứ tự dòng)
 * @param configs: Map chứa các config cần lưu
 */
void SubtitleConfig::saveConfigForSubtitleItem(int itemId, const std::map<QString, QVariant> &configs)
{
    QSettings settings("haidanghth910", "srteditor");
    settings.beginGroup(QString("SubtitleItems/%1").arg(itemId));
    for (const auto &pair : configs)
    {
        settings.setValue(pair.first, pair.second);
    }
    settings.endGroup();
}

/**
 * Đọc config cho một subtitle item từ QSettings group.
 * @param itemId: Định danh subtitle item
 * @return: Map chứa các config đã lưu
 */
std::map<QString, QVariant> SubtitleConfig::loadConfigForSubtitleItem(int itemId)
{
    QSettings settings("haidanghth910", "srteditor");
    std::map<QString, QVariant> configs;
    settings.beginGroup(QString("SubtitleItems/%1").arg(itemId));
    QStringList keys = settings.childKeys();
    for (const QString &key : keys)
    {
        configs[key] = settings.value(key);
    }
    settings.endGroup();
    return configs;
}

void SubtitleConfig::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (m_initialized)
        return; // chỉ chạy 1 lần cho instance này
    QSettings settings("haidanghth910", "srteditor");

    // (tuỳ bạn) có thể xoá/invalid các key cache chi tiết ở đây nếu cần
    std::string translateProvider =
        settings.value("translate/provider", "OpenAI").toString().toStdString();
    if (translateProvider == "OpenAI")
        this->getOpenaiChatModels();
    else if (translateProvider == "Github Models")
        this->getGithubChatModels();

    std::string speechProvider =
        settings.value("tts/provider", "OpenAI").toString().toStdString();
    if (speechProvider == "OpenAI")
    {
        this->getOpenaiSpeechModels();
        // Load dữ liệu từ cache lên UI
        ui->speechVoices->addItems(this->openaiSpeechVoices);
        ui->speechModels->addItems(this->openaiSpeechModels);
    }
    else if (speechProvider == "ElevenLabs")
    {
        this->getElevenlabsSpeechModels();
        this->getElevenlabsSpeechVoices();
        // Load dữ liệu từ cache lên UI
        ui->speechModels->addItems(this->elevenlabsSpeechModels);
        ui->speechVoices->addItems(this->elevenlabsSpeechVoices);
    }
    
    // Load dữ liệu translate models lên UI
    if (translateProvider == "OpenAI") {
        ui->translateModels->addItems(this->openaiChatModels);
    } else if (translateProvider == "Github Models") {
        ui->translateModels->addItems(this->githubChatModels);
    }

    m_initialized = true;
}

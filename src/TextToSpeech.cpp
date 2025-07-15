#include "TextToSpeech.h"

TextToSpeech::TextToSpeech()
{
}

TextToSpeech::~TextToSpeech()
{
}

static size_t writeCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total = size * nmemb;
    static_cast<std::string*>(userdata)->append((char*)ptr, total);
    return total;
}

void TextToSpeech::openaiTextToSpeech(std::string text, std::string outputDir, float speed, std::string model, std::string voice, std::string instructions, std::string outputFile, std::string apiKey)
{
    // Tạo JSON body
    nlohmann::json bodyJson = {
        {"model", model},
        {"input", text},
        {"voice", voice},
        {"instructions", instructions}
    };
    std::string body = bodyJson.dump();

    CURL *curl = curl_easy_init();
    if (!curl) return;

    struct curl_slist *headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/audio/speech");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        // Xác định tên file đầu ra
        std::string outPath;
        if (outputFile.empty()) {
            auto now = std::chrono::system_clock::now();
            auto t = std::chrono::system_clock::to_time_t(now);
            std::tm tm = *std::localtime(&t);
            std::stringstream ss;
            ss << outputDir << "/speech_";
            ss << std::put_time(&tm, "%Y%m%d_%H%M%S");
            ss << ".mp3";
            outPath = ss.str();
        } else {
            outPath = outputDir + "/" + outputFile;
        }
        std::ofstream outFileStream(outPath, std::ios::binary);
        if (outFileStream.is_open()) {
            outFileStream.write(response.c_str(), response.size());
            outFileStream.close();
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void TextToSpeech::elevenlabsTextToSpeech(std::string text, std::string outputDir, std::string voiceId, std::string modelId, std::string outputFormat, std::string outputFile, std::string apiKey)
{
    // Tạo endpoint
    std::string url = "https://api.elevenlabs.io/v1/text-to-speech/" + voiceId;

    // Tạo JSON body
    nlohmann::json bodyJson = {
        {"text", text},
        {"model_id", modelId},
        {"output_format", outputFormat}
    };
    std::string body = bodyJson.dump();

    CURL *curl = curl_easy_init();
    if (!curl) return;

    struct curl_slist *headers = nullptr;
    std::string authHeader = "xi-api-key: " + apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        // Xác định tên file đầu ra
        std::string outPath;
        if (outputFile.empty()) {
            auto now = std::chrono::system_clock::now();
            auto t = std::chrono::system_clock::to_time_t(now);
            std::tm tm = *std::localtime(&t);
            std::stringstream ss;
            ss << outputDir << "/speech_";
            ss << std::put_time(&tm, "%Y%m%d_%H%M%S");
            ss << ".mp3";
            outPath = ss.str();
        } else {
            outPath = outputDir + "/" + outputFile;
        }
        std::ofstream outFileStream(outPath, std::ios::binary);
        if (outFileStream.is_open()) {
            outFileStream.write(response.c_str(), response.size());
            outFileStream.close();
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

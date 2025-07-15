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

std::string Translator::openaiTranslate(std::string subtitle, std::string srcLang, std::string dstLang, std::string apiKey, float temperature, float topp, int maxTokens)
{
    // 1. Tạo prompt
    std::ostringstream oss;
    oss << "You are a professional translator. "
        << "Translate all user input from " << srcLang << " to " << dstLang << " accurately and naturally. "
        << "Do not explain. Do not interpret. Only return the translated text. "
        << "Preserve formatting such as line breaks, punctuation, and special characters.";
    std::string systemPrompt = oss.str();

    // 3. Tạo JSON payload
    nlohmann::json payload = {
        {"model", "gpt-4.1"},
        {"input", {
            {
                {"role", "system"},
                {"content", {
                    {
                        {"type", "input_text"},
                        {"text", systemPrompt}  // std::string hoặc QString.toStdString()
                    }
                }}
            },
            {
                {"role", "user"},
                {"content", {
                    {
                        {"type", "input_text"},
                        {"text", subtitle}  // nội dung cần dịch, std::string
                    }
                }}
            }
        }},
        {"text", {
            {"format", {
                {"type", "text"}
            }}
        }},
        {"reasoning", nlohmann::json::object()},
        {"tools", nlohmann::json::array()},
        {"temperature", temperature},
        {"max_output_tokens", maxTokens},
        {"top_p", topp}
    };

    // 4. Gửi request bằng libcurl
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        // 4.2. Cấu hình libcurl
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/responses");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        std::string payloadStr = payload.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Ghi phản hồi vào response string
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return std::string("[OpenAI Error: ") + curl_easy_strerror(res) + "]";
        }
    }

    // 5. Parse phản hồi để lấy kết quả dịch
    try {
        auto j = nlohmann::json::parse(response);

        if (j.contains("output") && j["output"].is_string()) {
            return j["output"];
        } else {
            return "[Invalid OpenAI Response]";
        }
    } catch (const std::exception& e) {
        return "[JSON Parse Error: " + std::string(e.what()) + "]";
    }
}

std::string Translator::githubModelsTranslate(std::string subtitle, std::string srcLang, std::string dstLang, std::string token, float temperature, float topp, int maxTokens)
{
    CURL *curl = curl_easy_init();
    if (!curl) return "[Error: Failed to initialize CURL]";

    std::string responseBuffer;
    std::string apiUrl = "https://models.github.ai/inference/chat/completions";

    // Tạo JSON payload yêu cầu
    nlohmann::json payload = {
        {"model", "openai/gpt-4.1"},
        {"temperature", temperature},
        {"top_p", topp},
        {"messages", {
            {
                {"role", "system"},
                {"content", "You are a professional translator. Translate all user input from vietnamese to english accurately and naturally. Do not explain. Do not interpret. Only return the translated text. Preserve formatting such as line breaks, punctuation, and special characters."}
            },
            {
                {"role", "user"},
                {"content", subtitle}
            }
        }}
    };

    std::string jsonStr = payload.dump();

    // Thiết lập header
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string authHeader = "Authorization: Bearer " + token;
    headers = curl_slist_append(headers, authHeader.c_str());

    // Cấu hình CURL
    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());                         // URL của API
    curl_easy_setopt(curl, CURLOPT_POST, 1L);                                    // Sử dụng POST
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);                         // Gửi header
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());                // Gửi payload
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);               // Ghi phản hồi vào biến
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);                 // Ghi ra chuỗi

    CURLcode res = curl_easy_perform(curl);

    // Dọn dẹp
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return std::string("[GitHub Models Error: ") + curl_easy_strerror(res) + "]";
    }

    // Parse JSON để trích nội dung bản dịch
    try {
        auto json = nlohmann::json::parse(responseBuffer);
        return json["choices"][0]["message"]["content"].get<std::string>();
    } catch (...) {
        return "[Error: Failed to parse JSON response]";
    }
}

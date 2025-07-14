#include "subtitle_item.h"

/**
 * @brief Construct a new SubtitleItem widget.
 *
 * Problem/Objectives: Instantiate all child controls defined in the designer
 * file, ensure the row stays compact and assign a unique identifier so that
 * it can later be referenced by MainWindow (e.g. selection/removal).
 *
 * Solution/Implementation: Call setupUi, set a maximum height of 132 px and
 * generate a QUuid.
 */
SubtitleItem::SubtitleItem(QWidget* parent): QWidget(parent), ui(new Ui::SubtitleItem)
{
    ui->setupUi(this);
    setMaximumHeight(132);
    this->uuid = QUuid::createUuid();

    connect(ui->subtitleItemSettings, &QPushButton::clicked, this, [this](){
        this->subtitleConfig->show();
    });
}

/**
 * @brief Destructor – nothing to clean explicitly because Qt uses parent
 * ownership to delete child widgets automatically.
 */
SubtitleItem::~SubtitleItem()
{
}

size_t SubtitleConfig::writeCallback(void *ptr, size_t size, size_t nmemb, void *userData)
{
    size_t total = size * nmemb;
    static_cast<std::string *>(userData)->append((char *)ptr, total);
    return total;
}

/**
 * @brief Update the numeric order label displayed in the UI.
 */
void SubtitleItem::setOrder(int order)
{
    ui->orderLbl->setText(QString::fromStdString(std::to_string(order)));
}

/**
 * @brief Retrieve the unique identifier of this subtitle row.
 */
QUuid SubtitleItem::getUuid()
{
    return this->uuid;
}

/**
 * @brief Access the selection checkbox so external components can observe its state.
 */
QCheckBox* SubtitleItem::getSelectedCheckbox() const
{
    return ui->isSelected;
}

/**
 * @brief Getter implementations for time components & order.
 *
 * Each getter simply returns the current value from the corresponding Qt
 * control. They exist so that business-logic (validation, export, …) can work
 * with plain integers instead of querying UI elements directly.
 */
int SubtitleItem::getOrder() const {
    return ui->orderLbl->text().toInt();
}

int SubtitleItem::getStartHour() const {
    return ui->startHourLbl->value();
}
int SubtitleItem::getStartMinute() const {
    return ui->startMinuteLbl->value();
}
int SubtitleItem::getStartSecond() const {
    return ui->startSecondLbl->value();
}
int SubtitleItem::getStartMillisecond() const {
    return ui->startMiliSecondLbl->value();
}
int SubtitleItem::getEndHour() const {
    return ui->endHourLbl->value();
}
int SubtitleItem::getEndMinute() const {
    return ui->endMinuteLbl->value();
}
int SubtitleItem::getEndSecond() const {
    return ui->endSecondLbl->value();
}
int SubtitleItem::getEndMillisecond() const {
    return ui->endMiliSecondLbl->value();
}
QString SubtitleItem::getContent() const {
    return ui->subtitleContent->toPlainText();
}

/**
 * @brief Setter implementations for time components & content.
 *
 * They simply propagate the provided value to the appropriate spin box or
 * text edit. Keeping them isolated makes it easier to update UI behaviour in
 * one place if the underlying widgets change.
 */
void SubtitleItem::setStartHour(int hour) {
    ui->startHourLbl->setValue(hour);
}
void SubtitleItem::setStartMinute(int minute) {
    ui->startMinuteLbl->setValue(minute);
}
void SubtitleItem::setStartSecond(int second) {
    ui->startSecondLbl->setValue(second);
}
void SubtitleItem::setStartMillisecond(int ms) {
    ui->startMiliSecondLbl->setValue(ms);
}
void SubtitleItem::setEndHour(int hour) {
    ui->endHourLbl->setValue(hour);
}
void SubtitleItem::setEndMinute(int minute) {
    ui->endMinuteLbl->setValue(minute);
}
void SubtitleItem::setEndSecond(int second) {
    ui->endSecondLbl->setValue(second);
}
void SubtitleItem::setEndMillisecond(int ms) {
    ui->endMiliSecondLbl->setValue(ms);
}
void SubtitleItem::setContent(const QString &content) {
    ui->subtitleContent->setPlainText(content);
}

QString SubtitleItem::openaiTranslate(std::string srcLang, std::string dstLang, float temperature = 1.0f, float topp = 1.0f, int maxTokens = 1024)
{
    // 1. Tạo prompt
    std::ostringstream oss;
    oss << "You are a professional translator. "
        << "Translate all user input from " << srcLang << " to " << dstLang << " accurately and naturally. "
        << "Do not explain. Do not interpret. Only return the translated text. "
        << "Preserve formatting such as line breaks, punctuation, and special characters.";
    std::string systemPrompt = oss.str();

    // 2. Lấy nội dung cần dịch
    std::string inputText = this->ui->subtitleContent->toPlainText().toStdString();

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
                        {"text", inputText}  // nội dung cần dịch, std::string
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
        // 4.1. Lấy API key từ cấu hình Qt
        QSettings settings("haidanghth910", "srteditor");
        QString apiKey = settings.value("translate/apiKey").toString();
        std::string apiKeyStr = apiKey.toStdString();

        // 4.2. Cấu hình libcurl
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKeyStr).c_str());

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
            return "[OpenAI Error: " + QString::fromStdString(curl_easy_strerror(res)) + "]";
        }
    }

    // 5. Parse phản hồi để lấy kết quả dịch
    try {
        auto j = nlohmann::json::parse(response);

        if (j.contains("output") && j["output"].is_string()) {
            return QString::fromStdString(j["output"]);
        } else {
            return "[Invalid OpenAI Response]";
        }
    } catch (const std::exception& e) {
        return "[JSON Parse Error: " + QString::fromStdString(e.what()) + "]";
    }
}

QString SubtitleItem::githubModelTranslate()
{

}

QString SubtitleItem::translate()
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        std::map<QString, QVariant> subtitleItemConfig = this->subtitleConfig->getConfigs();

    };
    curl_easy_cleanup(curl);
}

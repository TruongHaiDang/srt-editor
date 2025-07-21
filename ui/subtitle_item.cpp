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

void SubtitleItem::translate()
{
    QSettings settings("haidanghth910", "srteditor");
    std::string translateProvider = settings.value("translate/provider", "OpenAI").toString().toStdString();
    std::string key = settings.value("translate/apiKey", "").toString().toStdString();
    
    std::string subtitle = ui->subtitleContent->toPlainText().toStdString();
    std::map<QString, QVariant> configs = this->subtitleConfig->getConfigs();

    Translator translator;
    std::string result;
    if (translateProvider == "OpenAI")
        result = translator.openaiTranslate(subtitle, configs["srcLang"].toString().toStdString(), configs["dstLang"].toString().toStdString(), key);
    else if (translateProvider == "Github Models")
        result = translator.githubModelsTranslate(subtitle, configs["srcLang"].toString().toStdString(), configs["dstLang"].toString().toStdString(), key);

    ui->subtitleContent->setText(QString::fromStdString(result));
}

void SubtitleItem::textToSpeech(std::string outputDir)
{
    QSettings settings("haidanghth910", "srteditor");
    std::string ttsProvider = settings.value("tts/provider", "OpenAI").toString().toStdString();
    std::string key = settings.value("tts/apiKey", "").toString().toStdString();

    std::string subtitle = ui->subtitleContent->toPlainText().toStdString();
    std::map<QString, QVariant> configs = this->subtitleConfig->getConfigs();
    
    TextToSpeech tts;
    std::string audioFilePath;
    if (ttsProvider == "OpenAI")
        audioFilePath = tts.openaiTextToSpeech(subtitle, outputDir, configs["speechModel"].toString().toStdString(), configs["speechVoiceName"].toString().toStdString(), configs["speechInstructions"].toString().toStdString(), "", key);
    else if (ttsProvider == "ElevenLabs")
        audioFilePath = tts.elevenlabsTextToSpeech(subtitle, outputDir, configs["speechVoiceId"].toString().toStdString(), configs["speechModel"].toString().toStdString(), configs["speechFileFormat"].toString().toStdString(), "", key);
    
    if (!audioFilePath.empty())
    {

    }
}

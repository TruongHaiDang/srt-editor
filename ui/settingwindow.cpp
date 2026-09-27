#include "settingwindow.h"

#include "elevenlabs.h"
#include "zalo.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtWidgets/QMessageBox>

#include <exception>

namespace
{
constexpr int kWindowWidth = 1200;
constexpr int kWindowHeight = 820;
constexpr int kTitleBarHeight = 32;
constexpr int kSidebarWidth = 180;
constexpr int kActionBarHeight = 52;
constexpr int kFormLabelWidth = 130;
constexpr int kSmallInputWidth = 92;
constexpr int kDelayInputWidth = 84;
constexpr int kPreviewHeight = 84;
constexpr int kButtonWidth = 92;
constexpr int kButtonHeight = 28;
constexpr int kSliderMinimum = 0;
constexpr int kSliderMaximum = 100;
constexpr double kSliderValueDivisor = 100.0;

const char* kWindowTitle = "Settings";
const char* kEmptyText = "";

const char* kSettingsGroup = "tts/elevenlabs";
const char* kApiKeySetting = "apiKey";
const char* kProviderSettingsGroup = "tts";
const char* kProviderSetting = "provider";
const char* kElevenLabsProvider = "elevenlabs";
const char* kZaloProvider = "zalo";
const char* kZaloSettingsGroup = "tts/zalo";
const char* kZaloApiKeySetting = "apiKey";
const char* kZaloSpeakerIdSetting = "speakerId";
const char* kZaloSpeedSetting = "speed";
const char* kZaloOutputFormatSetting = "outputFormat";
const char* kZaloPreviewTextSetting = "previewText";
const char* kOpenAISettingsGroup = "translator/openai";
const char* kOpenAIApiKeySetting = "apiKey";
const char* kModelSetting = "model";
const char* kVoiceSetting = "voice";
const char* kVoiceIdSetting = "voiceId";
const char* kOutputFormatSetting = "outputFormat";
const char* kStabilitySetting = "stability";
const char* kSimilaritySetting = "similarity";
const char* kStyleSetting = "style";
const char* kSpeakerBoostSetting = "speakerBoost";
const char* kOutputFolderSetting = "outputFolder";
const char* kFilePatternSetting = "filePattern";
const char* kMaxCharsSetting = "maxCharsPerRequest";
const char* kDelaySetting = "delayMs";
const char* kPreviewTextSetting = "previewText";

const char* kDefaultOutputFormat = "mp3_44100_128";
const char* kDefaultFilePattern = "{filename}_{voice}_{index}";
constexpr int kDefaultStability = 50;
constexpr int kDefaultSimilarity = 75;
constexpr int kDefaultStyle = 0;
constexpr bool kDefaultSpeakerBoost = true;
constexpr int kDefaultMaxCharsPerRequest = 5000;
constexpr int kDefaultDelayMs = 250;
constexpr int kDefaultZaloSpeakerId = 1;
constexpr int kDefaultZaloSpeed = 100;
constexpr int kZaloMinimumSpeed = 80;
constexpr int kZaloMaximumSpeed = 120;
constexpr int kZaloSpeedDivisor = 100;
constexpr int kZaloWavEncoding = 0;
constexpr int kZaloMp3Encoding = 1;

struct VoiceOption final
{
    QString name;
    QString voice_id;
};

QStringList extractModelIds(const QString& responseBody)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(responseBody.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return {};
    }

    const QJsonArray models = document.isArray()
        ? document.array()
        : document.object().value("models").toArray();

    QStringList modelIds;
    for (const QJsonValue& value : models) {
        const QJsonObject model = value.toObject();
        const QString modelId = model.value("model_id").toString();
        if (!modelId.isEmpty()) {
            modelIds.append(modelId);
        }
    }

    modelIds.removeDuplicates();
    return modelIds;
}

QList<VoiceOption> extractVoiceOptions(const QString& responseBody)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(responseBody.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }

    QList<VoiceOption> voiceOptions;
    const QJsonArray voices = document.object().value("voices").toArray();
    for (const QJsonValue& value : voices) {
        const QJsonObject voice = value.toObject();
        const QString voiceId = voice.value("voice_id").toString();
        const QString name = voice.value("name").toString();
        if (voiceId.isEmpty() || name.isEmpty()) {
            continue;
        }

        voiceOptions.append(VoiceOption{name, voiceId});
    }

    return voiceOptions;
}

QString currentVoiceId(const QComboBox& comboBox)
{
    const QString voiceId = comboBox.currentData().toString().trimmed();
    if (!voiceId.isEmpty()) {
        return voiceId;
    }

    return comboBox.currentText().trimmed();
}

QString extractZaloAudioUrl(const std::string& responseBody, QString& errorMessage)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(
        QByteArray(responseBody.data(), static_cast<qsizetype>(responseBody.size())),
        &parseError
    );
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        errorMessage = "Zalo returned an invalid JSON response.";
        return {};
    }

    const QJsonObject response = document.object();
    const int errorCode = response.value("error_code").toInt(-1);
    if (errorCode != 0) {
        const QString apiMessage = response.value("error_message").toString();
        errorMessage = apiMessage.isEmpty()
            ? QString("Zalo API error %1.").arg(errorCode)
            : QString("Zalo API error %1: %2").arg(errorCode).arg(apiMessage);
        return {};
    }

    const QString audioUrl = response.value("data").toObject().value("url").toString().trimmed();
    if (audioUrl.isEmpty()) {
        errorMessage = "Zalo response does not contain an audio URL.";
    }
    return audioUrl;
}
}

SettingWindow::SettingWindow(QWidget* parent)
    : QDialog(parent)
{
    buildUi();
    loadSettings();
    applyStyle();

    previewAudioOutput_ = new QAudioOutput(this);
    previewPlayer_ = new QMediaPlayer(this);
    previewAudioBuffer_ = new QBuffer(this);
    previewPlayer_->setAudioOutput(previewAudioOutput_);
    connect(previewPlayer_, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString& errorText) {
        if (error != QMediaPlayer::NoError) {
            QMessageBox::critical(this, "TTS Preview", QString("Cannot play preview: %1").arg(errorText));
        }
    });
}

bool SettingWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != titleBar_) {
        return QDialog::eventFilter(watched, event);
    }

    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            dragPosition_ = mouseEvent->globalPosition().toPoint() - frameGeometry().topLeft();
            return true;
        }
    }

    if (event->type() == QEvent::MouseMove && !dragPosition_.isNull()) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->buttons() & Qt::LeftButton) {
            move(mouseEvent->globalPosition().toPoint() - dragPosition_);
            return true;
        }
    }

    if (event->type() == QEvent::MouseButtonRelease) {
        dragPosition_ = QPoint();
        return true;
    }

    return QDialog::eventFilter(watched, event);
}

void SettingWindow::buildUi()
{
    setObjectName("ttsWindow");
    setWindowTitle(kWindowTitle);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    resize(kWindowWidth, kWindowHeight);
    setMinimumSize(960, 640);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    buildTitleBar(*rootLayout);
    buildContent(*rootLayout);
    buildActionBar(*rootLayout);
}

void SettingWindow::buildTitleBar(QVBoxLayout& rootLayout)
{
    titleBar_ = new QWidget(this);
    titleBar_->setObjectName("ttsTitleBar");
    titleBar_->setFixedHeight(kTitleBarHeight);
    titleBar_->installEventFilter(this);

    auto* titleLayout = new QHBoxLayout(titleBar_);
    titleLayout->setContentsMargins(8, 0, 8, 0);
    titleLayout->setSpacing(8);

    auto* iconLabel = new QLabel("⚙", titleBar_);
    iconLabel->setObjectName("ttsTitleIcon");
    titleLayout->addWidget(iconLabel);

    auto* titleLabel = new QLabel(kWindowTitle, titleBar_);
    titleLabel->setObjectName("ttsTitleLabel");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    auto* closeButton = new QPushButton("×", titleBar_);
    closeButton->setObjectName("ttsCloseButton");
    closeButton->setFixedSize(28, 24);
    connect(closeButton, &QPushButton::clicked, this, &SettingWindow::reject);
    titleLayout->addWidget(closeButton);

    rootLayout.addWidget(titleBar_);
}

void SettingWindow::buildContent(QVBoxLayout& rootLayout)
{
    auto* content = new QWidget(this);
    content->setObjectName("ttsContent");

    auto* contentLayout = new QHBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    buildSidebar(*content, *contentLayout);
    buildSettingsPanel(*content, *contentLayout);

    rootLayout.addWidget(content, 1);
}

void SettingWindow::buildSidebar(QWidget& parent, QHBoxLayout& contentLayout)
{
    sidebar_ = new QListWidget(&parent);
    sidebar_->setObjectName("ttsSidebar");
    sidebar_->setFixedWidth(kSidebarWidth);
    sidebar_->setFocusPolicy(Qt::NoFocus);
    sidebar_->addItem("ElevenLabs API");
    sidebar_->addItem("Zalo AI");
    sidebar_->addItem("Audio output");
    sidebar_->addItem("OpenAI API");
    sidebar_->setCurrentRow(0);
    connect(sidebar_, &QListWidget::currentRowChanged, this, [this](int row) {
        if (settingsStack_ == nullptr || row < 0 || row >= settingsStack_->count()) {
            return;
        }

        settingsStack_->setCurrentIndex(row);
    });

    contentLayout.addWidget(sidebar_);
}

void SettingWindow::buildSettingsPanel(QWidget& parent, QHBoxLayout& contentLayout)
{
    auto* panelContainer = new QWidget(&parent);
    panelContainer->setObjectName("ttsSettingsContainer");
    auto* panelLayout = new QVBoxLayout(panelContainer);
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->setSpacing(0);
    buildProviderSelector(*panelLayout);

    settingsStack_ = new QStackedWidget(panelContainer);
    settingsStack_->setObjectName("ttsSettingsStack");

    int row = 0;
    QGridLayout* formLayout = nullptr;
    auto* elevenLabsPage = createSettingsScrollPage(parent, formLayout);
    auto* elevenLabsPanel = elevenLabsPage->widget();
    addElevenLabsSection(*formLayout, *elevenLabsPanel, row);
    addVoiceOverridesSection(*formLayout, *elevenLabsPanel, row);
    addPreviewSection(*formLayout, *elevenLabsPanel, row);
    settingsStack_->addWidget(elevenLabsPage);

    row = 0;
    formLayout = nullptr;
    auto* zaloPage = createSettingsScrollPage(parent, formLayout);
    auto* zaloPanel = zaloPage->widget();
    addZaloSection(*formLayout, *zaloPanel, row);
    addZaloPreviewSection(*formLayout, *zaloPanel, row);
    settingsStack_->addWidget(zaloPage);

    row = 0;
    formLayout = nullptr;
    auto* outputPage = createSettingsScrollPage(parent, formLayout);
    auto* outputPanel = outputPage->widget();
    addProcessingSection(*formLayout, *outputPanel, row);
    settingsStack_->addWidget(outputPage);

    row = 0;
    formLayout = nullptr;
    auto* openAiPage = createSettingsScrollPage(parent, formLayout);
    auto* openAiPanel = openAiPage->widget();
    addOpenAISection(*formLayout, *openAiPanel, row);
    settingsStack_->addWidget(openAiPage);

    panelLayout->addWidget(settingsStack_, 1);
    contentLayout.addWidget(panelContainer, 1);
}

void SettingWindow::buildProviderSelector(QVBoxLayout& panelLayout)
{
    auto* selector = new QWidget(this);
    selector->setObjectName("ttsProviderSelector");
    auto* layout = new QHBoxLayout(selector);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(12);

    auto* label = new QLabel("Audio provider", selector);
    label->setObjectName("ttsProviderLabel");
    providerComboBox_ = createComboBox({}, *selector);
    providerComboBox_->setMinimumWidth(180);
    providerComboBox_->addItem("ElevenLabs", kElevenLabsProvider);
    providerComboBox_->addItem("Zalo AI", kZaloProvider);
    providerSummaryLabel_ = new QLabel(selector);
    providerSummaryLabel_->setObjectName("ttsProviderSummary");

    layout->addWidget(label);
    layout->addWidget(providerComboBox_);
    layout->addWidget(providerSummaryLabel_, 1);
    panelLayout.addWidget(selector);

    connect(providerComboBox_, &QComboBox::currentIndexChanged, this, [this](int) {
        updateProviderSummary();
        if (sidebar_ != nullptr) {
            sidebar_->setCurrentRow(providerComboBox_->currentData().toString() == kZaloProvider ? 1 : 0);
        }
    });
    updateProviderSummary();
}

void SettingWindow::buildActionBar(QVBoxLayout& rootLayout)
{
    auto* actionBar = new QWidget(this);
    actionBar->setObjectName("ttsActionBar");
    actionBar->setFixedHeight(kActionBarHeight);

    auto* actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setContentsMargins(16, 0, 16, 0);
    actionLayout->setSpacing(8);
    actionLayout->addStretch();

    auto* saveButton = createButton("Save", *actionBar);
    auto* cancelButton = createButton("Cancel", *actionBar);
    auto* applyButton = createButton("Apply", *actionBar);

    connect(saveButton, &QPushButton::clicked, this, &SettingWindow::saveAndAccept);
    connect(cancelButton, &QPushButton::clicked, this, &SettingWindow::reject);
    connect(applyButton, &QPushButton::clicked, this, &SettingWindow::saveSettings);

    actionLayout->addWidget(saveButton);
    actionLayout->addWidget(cancelButton);
    actionLayout->addWidget(applyButton);

    rootLayout.addWidget(actionBar);
}

void SettingWindow::applyStyle()
{
    QFile file(":/css/ttswindow.qss");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    setStyleSheet(stream.readAll());
}

void SettingWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup(kProviderSettingsGroup);
    if (providerComboBox_ != nullptr) {
        const QString provider = settings.value(kProviderSetting, kElevenLabsProvider).toString();
        const int providerIndex = providerComboBox_->findData(provider);
        providerComboBox_->setCurrentIndex(providerIndex >= 0 ? providerIndex : 0);
        updateProviderSummary();
    }
    settings.endGroup();

    settings.beginGroup(kSettingsGroup);

    if (apiKeyEdit_ != nullptr) {
        apiKeyEdit_->setText(settings.value(kApiKeySetting).toString());
    }

    if (modelComboBox_ != nullptr) {
        restoreComboBoxValue(*modelComboBox_, settings.value(kModelSetting, kEmptyText).toString());
    }

    if (voiceComboBox_ != nullptr) {
        const QString savedVoiceId = settings.value(kVoiceIdSetting).toString().trimmed();
        const QString savedVoiceName = settings.value(kVoiceSetting, kEmptyText).toString().trimmed();
        if (!savedVoiceId.isEmpty() && !savedVoiceName.isEmpty()) {
            voiceComboBox_->addItem(savedVoiceName, savedVoiceId);
            voiceComboBox_->setCurrentIndex(voiceComboBox_->count() - 1);
        } else {
            restoreComboBoxValue(*voiceComboBox_, savedVoiceId.isEmpty() ? savedVoiceName : savedVoiceId);
        }
    }

    if (outputFormatComboBox_ != nullptr) {
        restoreComboBoxValue(*outputFormatComboBox_, settings.value(kOutputFormatSetting, kDefaultOutputFormat).toString());
    }

    if (stabilitySlider_ != nullptr) {
        stabilitySlider_->setValue(settings.value(kStabilitySetting, kDefaultStability).toInt());
    }

    if (similaritySlider_ != nullptr) {
        similaritySlider_->setValue(settings.value(kSimilaritySetting, kDefaultSimilarity).toInt());
    }

    if (styleSlider_ != nullptr) {
        styleSlider_->setValue(settings.value(kStyleSetting, kDefaultStyle).toInt());
    }

    if (speakerBoostCheckBox_ != nullptr) {
        speakerBoostCheckBox_->setChecked(settings.value(kSpeakerBoostSetting, kDefaultSpeakerBoost).toBool());
    }

    if (outputFolderEdit_ != nullptr) {
        outputFolderEdit_->setText(settings.value(kOutputFolderSetting, kEmptyText).toString());
    }

    if (filePatternEdit_ != nullptr) {
        filePatternEdit_->setText(settings.value(kFilePatternSetting, kDefaultFilePattern).toString());
    }

    if (maxCharsEdit_ != nullptr) {
        maxCharsEdit_->setText(settings.value(kMaxCharsSetting, kDefaultMaxCharsPerRequest).toString());
    }

    if (delayEdit_ != nullptr) {
        delayEdit_->setText(settings.value(kDelaySetting, kDefaultDelayMs).toString());
    }

    if (previewTextEdit_ != nullptr) {
        previewTextEdit_->setPlainText(settings.value(kPreviewTextSetting, kEmptyText).toString());
    }
    settings.endGroup();

    settings.beginGroup(kZaloSettingsGroup);
    if (zaloApiKeyEdit_ != nullptr) {
        zaloApiKeyEdit_->setText(settings.value(kZaloApiKeySetting).toString());
    }
    if (zaloSpeakerComboBox_ != nullptr) {
        const int speakerIndex = zaloSpeakerComboBox_->findData(
            settings.value(kZaloSpeakerIdSetting, kDefaultZaloSpeakerId).toInt()
        );
        if (speakerIndex >= 0) {
            zaloSpeakerComboBox_->setCurrentIndex(speakerIndex);
        }
    }
    if (zaloSpeedSlider_ != nullptr) {
        zaloSpeedSlider_->setValue(settings.value(kZaloSpeedSetting, kDefaultZaloSpeed).toInt());
    }
    if (zaloOutputFormatComboBox_ != nullptr) {
        const int formatIndex = zaloOutputFormatComboBox_->findData(
            settings.value(kZaloOutputFormatSetting, kZaloWavEncoding).toInt()
        );
        if (formatIndex >= 0) {
            zaloOutputFormatComboBox_->setCurrentIndex(formatIndex);
        }
    }
    if (zaloPreviewTextEdit_ != nullptr) {
        zaloPreviewTextEdit_->setPlainText(settings.value(kZaloPreviewTextSetting).toString());
    }
    settings.endGroup();

    settings.beginGroup(kOpenAISettingsGroup);
    if (openAiApiKeyEdit_ != nullptr) {
        openAiApiKeyEdit_->setText(settings.value(kOpenAIApiKeySetting).toString());
    }
    settings.endGroup();
}

void SettingWindow::saveSettings() const
{
    QSettings settings;
    settings.beginGroup(kProviderSettingsGroup);
    if (providerComboBox_ != nullptr) {
        settings.setValue(kProviderSetting, providerComboBox_->currentData().toString());
    }
    settings.endGroup();

    settings.beginGroup(kSettingsGroup);

    if (apiKeyEdit_ != nullptr) {
        settings.setValue(kApiKeySetting, apiKeyEdit_->text());
    }

    if (modelComboBox_ != nullptr) {
        settings.setValue(kModelSetting, modelComboBox_->currentText());
    }

    if (voiceComboBox_ != nullptr) {
        settings.setValue(kVoiceSetting, voiceComboBox_->currentText().trimmed());
        settings.setValue(kVoiceIdSetting, currentVoiceId(*voiceComboBox_));
    }

    if (outputFormatComboBox_ != nullptr) {
        settings.setValue(kOutputFormatSetting, outputFormatComboBox_->currentText());
    }

    if (stabilitySlider_ != nullptr) {
        settings.setValue(kStabilitySetting, stabilitySlider_->value());
    }

    if (similaritySlider_ != nullptr) {
        settings.setValue(kSimilaritySetting, similaritySlider_->value());
    }

    if (styleSlider_ != nullptr) {
        settings.setValue(kStyleSetting, styleSlider_->value());
    }

    if (speakerBoostCheckBox_ != nullptr) {
        settings.setValue(kSpeakerBoostSetting, speakerBoostCheckBox_->isChecked());
    }

    if (outputFolderEdit_ != nullptr) {
        settings.setValue(kOutputFolderSetting, outputFolderEdit_->text());
    }

    if (filePatternEdit_ != nullptr) {
        settings.setValue(kFilePatternSetting, filePatternEdit_->text());
    }

    if (maxCharsEdit_ != nullptr) {
        settings.setValue(kMaxCharsSetting, maxCharsEdit_->text());
    }

    if (delayEdit_ != nullptr) {
        settings.setValue(kDelaySetting, delayEdit_->text());
    }

    if (previewTextEdit_ != nullptr) {
        settings.setValue(kPreviewTextSetting, previewTextEdit_->toPlainText());
    }
    settings.endGroup();

    settings.beginGroup(kZaloSettingsGroup);
    if (zaloApiKeyEdit_ != nullptr) {
        settings.setValue(kZaloApiKeySetting, zaloApiKeyEdit_->text().trimmed());
    }
    if (zaloSpeakerComboBox_ != nullptr) {
        settings.setValue(kZaloSpeakerIdSetting, zaloSpeakerComboBox_->currentData().toInt());
    }
    if (zaloSpeedSlider_ != nullptr) {
        settings.setValue(kZaloSpeedSetting, zaloSpeedSlider_->value());
    }
    if (zaloOutputFormatComboBox_ != nullptr) {
        settings.setValue(kZaloOutputFormatSetting, zaloOutputFormatComboBox_->currentData().toInt());
    }
    if (zaloPreviewTextEdit_ != nullptr) {
        settings.setValue(kZaloPreviewTextSetting, zaloPreviewTextEdit_->toPlainText());
    }
    settings.endGroup();

    settings.beginGroup(kOpenAISettingsGroup);
    if (openAiApiKeyEdit_ != nullptr) {
        settings.setValue(kOpenAIApiKeySetting, openAiApiKeyEdit_->text());
    }
    settings.endGroup();
}

void SettingWindow::addElevenLabsSection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    auto* title = createSectionTitle("ElevenLabs API Settings", parent);
    formLayout.addWidget(title, row, 0, 1, 2);
    ++row;

    auto* topSeparator = createSeparator(parent);
    formLayout.addWidget(topSeparator, row, 0, 1, 2);
    ++row;

    apiKeyEdit_ = createLineEdit(QString(), parent, QLineEdit::Password);
    auto* loadModelsButton = createButton("Load models", parent);
    auto* apiKeyRow = new QWidget(&parent);
    auto* apiKeyLayout = new QHBoxLayout(apiKeyRow);
    apiKeyLayout->setContentsMargins(0, 0, 0, 0);
    apiKeyLayout->setSpacing(8);
    apiKeyLayout->addWidget(apiKeyEdit_, 1);
    apiKeyLayout->addWidget(loadModelsButton);
    connect(loadModelsButton, &QPushButton::clicked, this, &SettingWindow::testElevenLabsConnection);
    formLayout.addWidget(createFormLabel("API Key:", parent), row, 0);
    formLayout.addWidget(apiKeyRow, row, 1);
    ++row;

    modelComboBox_ = createComboBox({}, parent);
    formLayout.addWidget(createFormLabel("Model:", parent), row, 0);
    formLayout.addWidget(modelComboBox_, row, 1);
    ++row;

    voiceComboBox_ = createComboBox({}, parent);
    auto* loadVoicesButton = createButton("Load Voices", parent);
    auto* voiceRow = new QWidget(&parent);
    auto* voiceLayout = new QHBoxLayout(voiceRow);
    voiceLayout->setContentsMargins(0, 0, 0, 0);
    voiceLayout->setSpacing(8);
    voiceLayout->addWidget(voiceComboBox_, 1);
    voiceLayout->addWidget(loadVoicesButton);
    connect(loadVoicesButton, &QPushButton::clicked, this, &SettingWindow::loadElevenLabsVoices);
    formLayout.addWidget(createFormLabel("Voice:", parent), row, 0);
    formLayout.addWidget(voiceRow, row, 1);
    ++row;

    outputFormatComboBox_ = createComboBox({
        "mp3_44100_128",
        "mp3_44100_192",
        "pcm_44100",
        "ulaw_8000",
    }, parent);
    formLayout.addWidget(createFormLabel("Output Format:", parent), row, 0);
    formLayout.addWidget(outputFormatComboBox_, row, 1);
    ++row;
}

void SettingWindow::addOpenAISection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    auto* title = createSectionTitle("OpenAI API Settings", parent);
    formLayout.addWidget(title, row, 0, 1, 2);
    ++row;

    auto* topSeparator = createSeparator(parent);
    formLayout.addWidget(topSeparator, row, 0, 1, 2);
    ++row;

    openAiApiKeyEdit_ = createLineEdit(QString(), parent, QLineEdit::Password);
    openAiApiKeyEdit_->setPlaceholderText("OpenAI API key");
    formLayout.addWidget(createFormLabel("API Key:", parent), row, 0);
    formLayout.addWidget(openAiApiKeyEdit_, row, 1);
    ++row;
}

void SettingWindow::addZaloSection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    formLayout.addWidget(createSectionTitle("Zalo AI Text-to-Speech", parent), row, 0, 1, 2);
    ++row;
    formLayout.addWidget(createSeparator(parent), row, 0, 1, 2);
    ++row;

    zaloApiKeyEdit_ = createLineEdit(QString(), parent, QLineEdit::Password);
    zaloApiKeyEdit_->setPlaceholderText("Zalo AI API key");
    formLayout.addWidget(createFormLabel("API Key:", parent), row, 0);
    formLayout.addWidget(zaloApiKeyEdit_, row, 1);
    ++row;

    zaloSpeakerComboBox_ = createComboBox({}, parent);
    zaloSpeakerComboBox_->addItem("Southern female", 1);
    zaloSpeakerComboBox_->addItem("Northern female", 2);
    zaloSpeakerComboBox_->addItem("Southern male", 3);
    zaloSpeakerComboBox_->addItem("Northern male", 4);
    formLayout.addWidget(createFormLabel("Voice:", parent), row, 0);
    formLayout.addWidget(zaloSpeakerComboBox_, row, 1);
    ++row;

    auto* speedContainer = new QWidget(&parent);
    auto* speedLayout = new QHBoxLayout(speedContainer);
    speedLayout->setContentsMargins(0, 0, 0, 0);
    speedLayout->setSpacing(12);
    zaloSpeedSlider_ = new QSlider(Qt::Horizontal, speedContainer);
    zaloSpeedSlider_->setObjectName("zaloSpeedSlider");
    zaloSpeedSlider_->setRange(kZaloMinimumSpeed, kZaloMaximumSpeed);
    zaloSpeedSlider_->setValue(kDefaultZaloSpeed);
    auto* speedValueLabel = new QLabel("1.00x", speedContainer);
    speedValueLabel->setObjectName("sliderValueLabel");
    speedValueLabel->setFixedWidth(48);
    speedValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    connect(zaloSpeedSlider_, &QSlider::valueChanged, speedValueLabel, [speedValueLabel](int value) {
        speedValueLabel->setText(QString::number(value / 100.0, 'f', 2) + "x");
    });
    speedLayout->addWidget(zaloSpeedSlider_, 1);
    speedLayout->addWidget(speedValueLabel);
    formLayout.addWidget(createFormLabel("Speed:", parent), row, 0);
    formLayout.addWidget(speedContainer, row, 1);
    ++row;

    zaloOutputFormatComboBox_ = createComboBox({}, parent);
    zaloOutputFormatComboBox_->addItem("WAV", kZaloWavEncoding);
    zaloOutputFormatComboBox_->addItem("MP3", kZaloMp3Encoding);
    formLayout.addWidget(createFormLabel("Output Format:", parent), row, 0);
    formLayout.addWidget(zaloOutputFormatComboBox_, row, 1);
    ++row;
}

void SettingWindow::addZaloPreviewSection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    formLayout.addWidget(createSeparator(parent), row, 0, 1, 2);
    ++row;

    zaloPreviewTextEdit_ = new QTextEdit(&parent);
    zaloPreviewTextEdit_->setObjectName("previewTextEdit");
    zaloPreviewTextEdit_->setFixedHeight(kPreviewHeight);
    zaloPreviewTextEdit_->setPlaceholderText("Nhập nội dung tiếng Việt để nghe thử");

    playZaloPreviewButton_ = createButton("▷  Play Preview", parent);
    connect(playZaloPreviewButton_, &QPushButton::clicked, this, &SettingWindow::playZaloPreview);

    auto* previewContent = new QWidget(&parent);
    auto* previewLayout = new QVBoxLayout(previewContent);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->setSpacing(8);
    previewLayout->addWidget(zaloPreviewTextEdit_);
    previewLayout->addWidget(playZaloPreviewButton_, 0, Qt::AlignRight);

    formLayout.addWidget(createFormLabel("Preview:", parent), row, 0, Qt::AlignTop);
    formLayout.addWidget(previewContent, row, 1);
    ++row;
}

void SettingWindow::addVoiceOverridesSection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    formLayout.addWidget(createSeparator(parent), row, 0, 1, 2);
    ++row;

    formLayout.addWidget(createSectionTitle("Voice Overrides", parent), row, 0, 1, 2);
    ++row;

    formLayout.addWidget(createFormLabel("Stability:", parent), row, 0);
    formLayout.addWidget(createSliderRow("stabilitySlider", kDefaultStability, parent, stabilitySlider_), row, 1);
    ++row;

    formLayout.addWidget(createFormLabel("Similarity:", parent), row, 0);
    formLayout.addWidget(createSliderRow("similaritySlider", kDefaultSimilarity, parent, similaritySlider_), row, 1);
    ++row;

    formLayout.addWidget(createFormLabel("Style:", parent), row, 0);
    formLayout.addWidget(createSliderRow("styleSlider", kDefaultStyle, parent, styleSlider_), row, 1);
    ++row;

    speakerBoostCheckBox_ = new QCheckBox("Speaker Boost", &parent);
    speakerBoostCheckBox_->setObjectName("speakerBoostCheckBox");
    speakerBoostCheckBox_->setChecked(kDefaultSpeakerBoost);
    formLayout.addWidget(speakerBoostCheckBox_, row, 1);
    ++row;
}

void SettingWindow::addProcessingSection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    formLayout.addWidget(createSeparator(parent), row, 0, 1, 2);
    ++row;

    formLayout.addWidget(createSectionTitle("Processing & Output", parent), row, 0, 1, 2);
    ++row;

    outputFolderEdit_ = createLineEdit(kEmptyText, parent);
    auto* browseButton = createButton("Browse...", parent);
    connect(browseButton, &QPushButton::clicked, this, &SettingWindow::browseOutputFolder);

    auto* outputFolderRow = new QWidget(&parent);
    auto* outputFolderLayout = new QHBoxLayout(outputFolderRow);
    outputFolderLayout->setContentsMargins(0, 0, 0, 0);
    outputFolderLayout->setSpacing(8);
    outputFolderLayout->addWidget(outputFolderEdit_, 1);
    outputFolderLayout->addWidget(browseButton);

    formLayout.addWidget(createFormLabel("Output Folder:", parent), row, 0);
    formLayout.addWidget(outputFolderRow, row, 1);
    ++row;

    formLayout.addWidget(createFormLabel("File Pattern:", parent), row, 0);
    filePatternEdit_ = createLineEdit(kDefaultFilePattern, parent);
    filePatternEdit_->setPlaceholderText(kDefaultFilePattern);
    filePatternEdit_->setToolTip(createFilePatternToolTip());
    formLayout.addWidget(filePatternEdit_, row, 1);
    ++row;

    auto* limitsRow = new QWidget(&parent);
    auto* limitsLayout = new QHBoxLayout(limitsRow);
    limitsLayout->setContentsMargins(0, 0, 0, 0);
    limitsLayout->setSpacing(16);

    maxCharsEdit_ = createLineEdit(QString::number(kDefaultMaxCharsPerRequest), parent);
    maxCharsEdit_->setFixedWidth(kSmallInputWidth);
    auto* delayLabel = new QLabel("Delay (ms):", limitsRow);
    delayLabel->setObjectName("ttsInlineLabel");
    delayEdit_ = createLineEdit(QString::number(kDefaultDelayMs), parent);
    delayEdit_->setFixedWidth(kDelayInputWidth);

    limitsLayout->addWidget(maxCharsEdit_);
    limitsLayout->addWidget(delayLabel);
    limitsLayout->addWidget(delayEdit_);
    limitsLayout->addStretch();

    formLayout.addWidget(createFormLabel("Max Chars/Req:", parent), row, 0);
    formLayout.addWidget(limitsRow, row, 1);
    ++row;
}

void SettingWindow::addPreviewSection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    formLayout.addWidget(createSeparator(parent), row, 0, 1, 2);
    ++row;

    previewTextEdit_ = new QTextEdit(&parent);
    previewTextEdit_->setObjectName("previewTextEdit");
    previewTextEdit_->setFixedHeight(kPreviewHeight);
    previewTextEdit_->setPlainText(kEmptyText);

    playPreviewButton_ = createButton("▷  Play Preview", parent);
    connect(playPreviewButton_, &QPushButton::clicked, this, &SettingWindow::playElevenLabsPreview);
    auto* previewContent = new QWidget(&parent);
    auto* previewLayout = new QVBoxLayout(previewContent);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->setSpacing(8);
    previewLayout->addWidget(previewTextEdit_);
    previewLayout->addWidget(playPreviewButton_, 0, Qt::AlignRight);

    formLayout.addWidget(createFormLabel("Preview:", parent), row, 0, Qt::AlignTop);
    formLayout.addWidget(previewContent, row, 1);
    ++row;
}

QLabel* SettingWindow::createFormLabel(const QString& text, QWidget& parent) const
{
    auto* label = new QLabel(text, &parent);
    label->setObjectName("ttsFormLabel");
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return label;
}

QLabel* SettingWindow::createSectionTitle(const QString& text, QWidget& parent) const
{
    auto* label = new QLabel(text, &parent);
    label->setObjectName("ttsSectionTitle");
    return label;
}

QFrame* SettingWindow::createSeparator(QWidget& parent) const
{
    auto* line = new QFrame(&parent);
    line->setObjectName("ttsSeparator");
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    return line;
}

QScrollArea* SettingWindow::createSettingsScrollPage(QWidget& parent, QGridLayout*& formLayout) const
{
    auto* scrollArea = new QScrollArea(&parent);
    scrollArea->setObjectName("ttsSettingsScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* panel = new QWidget(scrollArea);
    panel->setObjectName("ttsSettingsPanel");

    auto* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(16, 16, 16, 16);
    panelLayout->setSpacing(12);

    formLayout = new QGridLayout();
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setHorizontalSpacing(16);
    formLayout->setVerticalSpacing(12);
    formLayout->setColumnMinimumWidth(0, kFormLabelWidth);
    formLayout->setColumnStretch(1, 1);

    panelLayout->addLayout(formLayout);
    panelLayout->addStretch();
    scrollArea->setWidget(panel);

    return scrollArea;
}

QLineEdit* SettingWindow::createLineEdit(const QString& text, QWidget& parent, QLineEdit::EchoMode echoMode) const
{
    auto* edit = new QLineEdit(text, &parent);
    edit->setObjectName("ttsLineEdit");
    edit->setEchoMode(echoMode);
    edit->setMinimumHeight(30);
    return edit;
}

QComboBox* SettingWindow::createComboBox(const QStringList& values, QWidget& parent) const
{
    auto* comboBox = new QComboBox(&parent);
    comboBox->setObjectName("ttsComboBox");
    comboBox->addItems(values);
    comboBox->setMinimumHeight(30);
    return comboBox;
}

QWidget* SettingWindow::createSliderRow(const QString& objectName, int initialValue, QWidget& parent, QSlider*& slider) const
{
    auto* container = new QWidget(&parent);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    slider = new QSlider(Qt::Horizontal, container);
    slider->setObjectName(objectName);
    slider->setRange(kSliderMinimum, kSliderMaximum);
    slider->setValue(initialValue);

    auto* valueLabel = new QLabel(QString("%1%").arg(initialValue), container);
    valueLabel->setObjectName("sliderValueLabel");
    valueLabel->setFixedWidth(40);
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    connect(slider, &QSlider::valueChanged, valueLabel, [valueLabel](int value) {
        valueLabel->setText(QString("%1%").arg(value));
    });

    layout->addWidget(slider, 1);
    layout->addWidget(valueLabel);
    return container;
}

QPushButton* SettingWindow::createButton(const QString& text, QWidget& parent) const
{
    auto* button = new QPushButton(text, &parent);
    button->setObjectName("ttsButton");
    button->setMinimumHeight(kButtonHeight);
    button->setMinimumWidth(kButtonWidth);
    return button;
}

QString SettingWindow::createFilePatternToolTip() const
{
    return QStringLiteral(
        "Available file pattern tokens:\n"
        "{voice} - voice name, e.g. Rachel.mp3\n"
        "{voice_id} - ElevenLabs voice ID, e.g. 21m00Tcm4TlvDq8ikWAM.mp3\n"
        "{model} - TTS model, e.g. eleven_multilingual_v2.mp3\n"
        "{date} - date, e.g. 20260520.mp3\n"
        "{time} - time, e.g. 194530.mp3\n"
        "{datetime} - timestamp, e.g. 20260520_194530.mp3\n"
        "{index} - sequence number, e.g. 0001.mp3\n"
        "{project} - project name, e.g. movie_subtitle.mp3\n"
        "{filename} - source filename, e.g. episode_01.mp3\n"
        "{text} - shortened preview text, e.g. hello_world.mp3\n"
        "{lang} - language, e.g. en.mp3\n"
        "{speaker} - speaker, e.g. narrator.mp3\n"
        "{chunk} - text chunk, e.g. chunk_03.mp3\n"
        "{uuid} - unique ID, e.g. 550e8400-e29b.mp3"
    );
}

void SettingWindow::saveAndAccept()
{
    saveSettings();
    accept();
}

void SettingWindow::updateProviderSummary()
{
    if (providerComboBox_ == nullptr || providerSummaryLabel_ == nullptr) {
        return;
    }

    const bool isZalo = providerComboBox_->currentData().toString() == kZaloProvider;
    providerSummaryLabel_->setText(
        isZalo
            ? "Convert and Convert all will use Zalo AI"
            : "Convert and Convert all will use ElevenLabs"
    );
}

void SettingWindow::restoreComboBoxValue(QComboBox& comboBox, const QString& value) const
{
    const int itemDataIndex = comboBox.findData(value);
    if (itemDataIndex >= 0) {
        comboBox.setCurrentIndex(itemDataIndex);
        return;
    }

    const int itemIndex = comboBox.findText(value);
    if (itemIndex >= 0) {
        comboBox.setCurrentIndex(itemIndex);
        return;
    }

    if (!value.isEmpty()) {
        comboBox.addItem(value);
        comboBox.setCurrentIndex(comboBox.count() - 1);
    }
}

void SettingWindow::browseOutputFolder()
{
    if (outputFolderEdit_ == nullptr) {
        return;
    }

    const QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "Select TTS Output Folder",
        outputFolderEdit_->text()
    );

    if (!folderPath.isEmpty()) {
        outputFolderEdit_->setText(folderPath);
    }
}

void SettingWindow::testElevenLabsConnection()
{
    if (apiKeyEdit_ == nullptr) {
        QMessageBox::warning(this, "ElevenLabs", "API key input is not available.");
        return;
    }

    const QString apiKey = apiKeyEdit_->text().trimmed();
    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, "ElevenLabs", "Please enter an ElevenLabs API key.");
        return;
    }

    auto* testButton = qobject_cast<QPushButton*>(sender());
    if (testButton != nullptr) {
        testButton->setEnabled(false);
    }

    try {
        ElevenLabsClient client(apiKey.toStdString());
        const ElevenLabsResponse response = client.getModels();

        if (response.status_code < 200 || response.status_code >= 300) {
            QMessageBox::warning(
                this,
                "ElevenLabs",
                QString("Connection failed. HTTP status: %1").arg(response.status_code)
            );
        } else {
            const QStringList modelIds = extractModelIds(QString::fromStdString(response.body));
            if (modelComboBox_ != nullptr && !modelIds.isEmpty()) {
                const QString currentModel = modelComboBox_->currentText();
                modelComboBox_->clear();
                modelComboBox_->addItems(modelIds);
                restoreComboBoxValue(*modelComboBox_, currentModel);
            }

            QMessageBox::information(this, "ElevenLabs", "Load models successful.");
        }
    } catch (const std::exception& error) {
        QMessageBox::critical(
            this,
            "ElevenLabs",
            QString("Load models failed: %1").arg(QString::fromUtf8(error.what()))
        );
    }

    if (testButton != nullptr) {
        testButton->setEnabled(true);
    }
}

void SettingWindow::loadElevenLabsVoices()
{
    if (apiKeyEdit_ == nullptr) {
        QMessageBox::warning(this, "ElevenLabs", "API key input is not available.");
        return;
    }

    const QString apiKey = apiKeyEdit_->text().trimmed();
    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, "ElevenLabs", "Please enter an ElevenLabs API key.");
        return;
    }

    auto* loadButton = qobject_cast<QPushButton*>(sender());
    if (loadButton != nullptr) {
        loadButton->setEnabled(false);
    }

    try {
        ElevenLabsClient client(apiKey.toStdString());
        const ElevenLabsResponse response = client.getVoices();

        if (response.status_code < 200 || response.status_code >= 300) {
            QMessageBox::warning(
                this,
                "ElevenLabs",
                QString("Load voices failed. HTTP status: %1").arg(response.status_code)
            );
        } else {
            const QList<VoiceOption> voiceOptions = extractVoiceOptions(QString::fromStdString(response.body));
            if (voiceOptions.isEmpty()) {
                QMessageBox::warning(this, "ElevenLabs", "No voices found in ElevenLabs response.");
            } else if (voiceComboBox_ != nullptr) {
                const QString currentVoice = currentVoiceId(*voiceComboBox_);
                voiceComboBox_->clear();

                for (const VoiceOption& voiceOption : voiceOptions) {
                    voiceComboBox_->addItem(voiceOption.name, voiceOption.voice_id);
                }

                restoreComboBoxValue(*voiceComboBox_, currentVoice);
                QMessageBox::information(this, "ElevenLabs", "Load voices successful.");
            }
        }
    } catch (const std::exception& error) {
        QMessageBox::critical(
            this,
            "ElevenLabs",
            QString("Load voices failed: %1").arg(QString::fromUtf8(error.what()))
        );
    }

    if (loadButton != nullptr) {
        loadButton->setEnabled(true);
    }
}

void SettingWindow::playElevenLabsPreview()
{
    if (apiKeyEdit_ == nullptr || modelComboBox_ == nullptr || voiceComboBox_ == nullptr ||
        stabilitySlider_ == nullptr || similaritySlider_ == nullptr || styleSlider_ == nullptr ||
        speakerBoostCheckBox_ == nullptr || previewTextEdit_ == nullptr || previewPlayer_ == nullptr ||
        previewAudioBuffer_ == nullptr) {
        QMessageBox::warning(this, "ElevenLabs Preview", "Preview controls are not available.");
        return;
    }

    const QString apiKey = apiKeyEdit_->text().trimmed();
    const QString modelId = modelComboBox_->currentText().trimmed();
    const QString voiceId = currentVoiceId(*voiceComboBox_);
    const QString previewText = previewTextEdit_->toPlainText().trimmed();
    if (apiKey.isEmpty() || modelId.isEmpty() || voiceId.isEmpty() || previewText.isEmpty()) {
        QMessageBox::warning(
            this,
            "ElevenLabs Preview",
            "API key, model, voice, and preview text are required."
        );
        return;
    }

    if (playPreviewButton_ != nullptr) {
        playPreviewButton_->setEnabled(false);
        playPreviewButton_->setText("Generating...");
    }

    try {
        ElevenLabsTextToSpeechRequest request;
        request.voice_id = voiceId.toStdString();
        request.text = previewText.toStdString();
        request.model_id = modelId.toStdString();
        request.output_format = kDefaultOutputFormat;
        request.voice_settings.stability = stabilitySlider_->value() / kSliderValueDivisor;
        request.voice_settings.similarity_boost = similaritySlider_->value() / kSliderValueDivisor;
        request.voice_settings.style = styleSlider_->value() / kSliderValueDivisor;
        request.voice_settings.use_speaker_boost = speakerBoostCheckBox_->isChecked();

        ElevenLabsClient client(apiKey.toStdString());
        const ElevenLabsResponse response = client.textToSpeech(request);
        if (response.status_code < 200 || response.status_code >= 300) {
            const QString errorBody = QString::fromUtf8(
                response.body.data(),
                static_cast<qsizetype>(response.body.size())
            ).left(500);
            QMessageBox::warning(
                this,
                "ElevenLabs Preview",
                QString("Generate preview failed. HTTP status: %1\n%2")
                    .arg(response.status_code)
                    .arg(errorBody)
            );
        } else if (response.body.empty()) {
            QMessageBox::warning(this, "ElevenLabs Preview", "ElevenLabs returned empty audio.");
        } else {
            previewPlayer_->stop();
            previewPlayer_->setSource(QUrl());
            previewAudioBuffer_->close();
            previewAudioBuffer_->setData(response.body.data(), static_cast<qsizetype>(response.body.size()));
            previewAudioBuffer_->open(QIODevice::ReadOnly);
            previewPlayer_->setSourceDevice(previewAudioBuffer_, QUrl("preview.mp3"));
            previewPlayer_->play();
        }
    } catch (const std::exception& error) {
        QMessageBox::critical(
            this,
            "ElevenLabs Preview",
            QString("Generate preview failed: %1").arg(QString::fromUtf8(error.what()))
        );
    }

    if (playPreviewButton_ != nullptr) {
        playPreviewButton_->setText("▷  Play Preview");
        playPreviewButton_->setEnabled(true);
    }
}

void SettingWindow::playZaloPreview()
{
    if (zaloApiKeyEdit_ == nullptr || zaloSpeakerComboBox_ == nullptr ||
        zaloSpeedSlider_ == nullptr || zaloOutputFormatComboBox_ == nullptr ||
        zaloPreviewTextEdit_ == nullptr || previewPlayer_ == nullptr ||
        previewAudioBuffer_ == nullptr) {
        QMessageBox::warning(this, "Zalo Preview", "Preview controls are not available.");
        return;
    }

    const QString apiKey = zaloApiKeyEdit_->text().trimmed();
    const QString previewText = zaloPreviewTextEdit_->toPlainText().trimmed();
    if (apiKey.isEmpty() || previewText.isEmpty()) {
        QMessageBox::warning(this, "Zalo Preview", "API key and preview text are required.");
        return;
    }

    playZaloPreviewButton_->setEnabled(false);
    playZaloPreviewButton_->setText("Generating...");

    try {
        ZaloTextToSpeechRequest request;
        request.text = previewText.toStdString();
        request.speaker_id = zaloSpeakerComboBox_->currentData().toInt();
        request.speed = static_cast<double>(zaloSpeedSlider_->value()) / kZaloSpeedDivisor;
        request.encoding = zaloOutputFormatComboBox_->currentData().toInt() == kZaloMp3Encoding
            ? ZaloAudioEncoding::Mp3
            : ZaloAudioEncoding::Wav;

        ZaloClient client(apiKey.toStdString());
        const ZaloResponse synthesisResponse = client.synthesize(request);
        if (synthesisResponse.status_code < 200 || synthesisResponse.status_code >= 300) {
            const QString errorBody = QString::fromUtf8(
                synthesisResponse.body.data(),
                static_cast<qsizetype>(synthesisResponse.body.size())
            ).left(500);
            QMessageBox::warning(
                this,
                "Zalo Preview",
                QString("Generate preview failed. HTTP status: %1\n%2")
                    .arg(synthesisResponse.status_code)
                    .arg(errorBody)
            );
        } else {
            QString apiError;
            const QString audioUrl = extractZaloAudioUrl(synthesisResponse.body, apiError);
            if (audioUrl.isEmpty()) {
                QMessageBox::warning(this, "Zalo Preview", apiError);
            } else {
                const ZaloResponse audioResponse = client.downloadAudio(audioUrl.toStdString());
                if (audioResponse.status_code < 200 || audioResponse.status_code >= 300) {
                    QMessageBox::warning(
                        this,
                        "Zalo Preview",
                        QString("Download preview failed. HTTP status: %1")
                            .arg(audioResponse.status_code)
                    );
                } else if (audioResponse.body.empty()) {
                    QMessageBox::warning(this, "Zalo Preview", "Zalo returned empty audio.");
                } else {
                    const bool isMp3 = request.encoding == ZaloAudioEncoding::Mp3;
                    previewPlayer_->stop();
                    previewPlayer_->setSource(QUrl());
                    previewAudioBuffer_->close();
                    previewAudioBuffer_->setData(
                        audioResponse.body.data(),
                        static_cast<qsizetype>(audioResponse.body.size())
                    );
                    previewAudioBuffer_->open(QIODevice::ReadOnly);
                    previewPlayer_->setSourceDevice(
                        previewAudioBuffer_,
                        QUrl(isMp3 ? "preview.mp3" : "preview.wav")
                    );
                    previewPlayer_->play();
                }
            }
        }
    } catch (const std::exception& error) {
        QMessageBox::critical(
            this,
            "Zalo Preview",
            QString("Generate preview failed: %1").arg(QString::fromUtf8(error.what()))
        );
    }

    playZaloPreviewButton_->setText("▷  Play Preview");
    playZaloPreviewButton_->setEnabled(true);
}

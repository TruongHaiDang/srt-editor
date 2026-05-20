#include "mainwindow.h"
#include "aboutwindow.h"
#include "elevenlabs.h"

#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtCore/QThread>
#include <QtCore/QUuid>

#include <qaction.h>
#include <qdebug.h>
#include <qfiledevice.h>
#include <qfiledialog.h>
#include <qmainwindow.h>
#include <qobject.h>
#include <qpicture.h>
#include <qstringconverter_base.h>

#include <exception>
#include <string>

namespace
{
constexpr int kTopBarHeight = 36;
constexpr int kStatusBarHeight = 24;
constexpr int kPropertiesPanelWidth = 270;
constexpr int kRowHeight = 28;

constexpr int kColumnIndex = 0;
constexpr int kColumnStartTime = 1;
constexpr int kColumnEndTime = 2;
constexpr int kColumnSubtitleText = 3;

const char* kAppTitle = "SubtitleEdit Free";
const char* kSettingsGroup = "tts/elevenlabs";
const char* kApiKeySetting = "apiKey";
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

const char* kDefaultModelId = "eleven_multilingual_v2";
const char* kDefaultOutputFormat = "mp3_44100_128";
const char* kDefaultFilePattern = "{filename}_{voice}_{index}";
constexpr int kDefaultStability = 50;
constexpr int kDefaultSimilarity = 75;
constexpr int kDefaultStyle = 0;
constexpr bool kDefaultSpeakerBoost = true;
constexpr int kDefaultMaxCharsPerRequest = 5000;
constexpr int kDefaultDelayMs = 250;
constexpr double kSliderValueDivisor = 100.0;

struct TextToSpeechSettings final
{
    QString apiKey;
    QString voiceId;
    QString modelId;
    QString outputFormat;
    QString outputFolder;
    QString filePattern;
    int maxCharsPerRequest = kDefaultMaxCharsPerRequest;
    int delayMs = kDefaultDelayMs;
    int stability = kDefaultStability;
    int similarity = kDefaultSimilarity;
    int style = kDefaultStyle;
    bool speakerBoost = kDefaultSpeakerBoost;
};

int readPositiveIntSetting(QSettings& settings, const char* key, int defaultValue)
{
    bool isValid = false;
    const int value = settings.value(key, defaultValue).toInt(&isValid);
    return isValid && value > 0 ? value : defaultValue;
}

int readNonNegativeIntSetting(QSettings& settings, const char* key, int defaultValue)
{
    bool isValid = false;
    const int value = settings.value(key, defaultValue).toInt(&isValid);
    return isValid && value >= 0 ? value : defaultValue;
}

int readSliderPercentSetting(QSettings& settings, const char* key, int defaultValue)
{
    bool isValid = false;
    const int value = settings.value(key, defaultValue).toInt(&isValid);
    return isValid && value >= 0 && value <= 100 ? value : defaultValue;
}

QString sanitizeFileName(QString value)
{
    value = value.trimmed();
    value.replace(QRegularExpression(R"([<>:"/\\|?*\x00-\x1F])"), "_");
    value.replace(QRegularExpression(R"(\s+)"), " ");
    return value.isEmpty() ? QStringLiteral("subtitle") : value;
}

QString fileExtensionForOutputFormat(const QString& outputFormat)
{
    if (outputFormat.startsWith("mp3", Qt::CaseInsensitive)) {
        return QStringLiteral("mp3");
    }

    if (outputFormat.startsWith("pcm", Qt::CaseInsensitive)) {
        return QStringLiteral("pcm");
    }

    if (outputFormat.startsWith("ulaw", Qt::CaseInsensitive)) {
        return QStringLiteral("ulaw");
    }

    return QStringLiteral("audio");
}

QString shortenForFileName(const QString& text)
{
    QString shortened = text.simplified();
    if (shortened.size() > 36) {
        shortened = shortened.left(36).trimmed();
    }

    return sanitizeFileName(shortened.replace(' ', '_')).toLower();
}

bool isLikelyElevenLabsVoiceId(const QString& value)
{
    static const QRegularExpression voiceIdPattern(R"(^[A-Za-z0-9_-]{10,}$)");
    return voiceIdPattern.match(value.trimmed()).hasMatch();
}

TextToSpeechSettings readTextToSpeechSettings()
{
    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    TextToSpeechSettings result;
    result.apiKey = settings.value(kApiKeySetting).toString().trimmed();
    result.voiceId = settings.value(kVoiceIdSetting).toString().trimmed();
    if (result.voiceId.isEmpty()) {
        const QString legacyVoiceValue = settings.value(kVoiceSetting).toString().trimmed();
        result.voiceId = isLikelyElevenLabsVoiceId(legacyVoiceValue) ? legacyVoiceValue : QString();
    }
    result.modelId = settings.value(kModelSetting, kDefaultModelId).toString().trimmed();
    result.outputFormat = settings.value(kOutputFormatSetting, kDefaultOutputFormat).toString().trimmed();
    result.outputFolder = settings.value(kOutputFolderSetting).toString().trimmed();
    result.filePattern = settings.value(kFilePatternSetting, kDefaultFilePattern).toString().trimmed();
    result.maxCharsPerRequest = readPositiveIntSetting(settings, kMaxCharsSetting, kDefaultMaxCharsPerRequest);
    result.delayMs = readNonNegativeIntSetting(settings, kDelaySetting, kDefaultDelayMs);
    result.stability = readSliderPercentSetting(settings, kStabilitySetting, kDefaultStability);
    result.similarity = readSliderPercentSetting(settings, kSimilaritySetting, kDefaultSimilarity);
    result.style = readSliderPercentSetting(settings, kStyleSetting, kDefaultStyle);
    result.speakerBoost = settings.value(kSpeakerBoostSetting, kDefaultSpeakerBoost).toBool();

    if (result.modelId.isEmpty()) {
        result.modelId = kDefaultModelId;
    }

    if (result.outputFormat.isEmpty()) {
        result.outputFormat = kDefaultOutputFormat;
    }

    if (result.filePattern.isEmpty()) {
        result.filePattern = kDefaultFilePattern;
    }

    return result;
}

ElevenLabsTextToSpeechRequest createTextToSpeechRequest(
    const TextToSpeechSettings& settings,
    const QString& text
)
{
    ElevenLabsTextToSpeechRequest request;
    request.voice_id = settings.voiceId.toStdString();
    request.text = text.toStdString();
    request.model_id = settings.modelId.toStdString();
    request.output_format = settings.outputFormat.toStdString();
    request.voice_settings.stability = settings.stability / kSliderValueDivisor;
    request.voice_settings.similarity_boost = settings.similarity / kSliderValueDivisor;
    request.voice_settings.style = settings.style / kSliderValueDivisor;
    request.voice_settings.use_speaker_boost = settings.speakerBoost;
    return request;
}

QString buildTextToSpeechOutputPath(
    const TextToSpeechSettings& settings,
    const QString& currentSrtFilePath,
    int rowIndex,
    const QString& text
)
{
    const QFileInfo sourceFile(currentSrtFilePath);
    const QString sourceBaseName = sourceFile.exists()
        ? sourceFile.completeBaseName()
        : QStringLiteral("subtitle");
    const QDateTime now = QDateTime::currentDateTime();

    QString fileName = settings.filePattern;
    fileName.replace("{voice}", settings.voiceId);
    fileName.replace("{voice_id}", settings.voiceId);
    fileName.replace("{model}", settings.modelId);
    fileName.replace("{date}", now.toString("yyyyMMdd"));
    fileName.replace("{time}", now.toString("HHmmss"));
    fileName.replace("{datetime}", now.toString("yyyyMMdd_HHmmss"));
    fileName.replace("{index}", QString("%1").arg(rowIndex, 4, 10, QChar('0')));
    fileName.replace("{project}", sourceBaseName);
    fileName.replace("{filename}", sourceBaseName);
    fileName.replace("{text}", shortenForFileName(text));
    fileName.replace("{lang}", "default");
    fileName.replace("{speaker}", settings.voiceId);
    fileName.replace("{chunk}", QString("chunk_%1").arg(rowIndex, 2, 10, QChar('0')));
    fileName.replace("{uuid}", QUuid::createUuid().toString(QUuid::WithoutBraces));
    fileName = sanitizeFileName(fileName);

    const QString extension = fileExtensionForOutputFormat(settings.outputFormat);
    if (!fileName.endsWith("." + extension, Qt::CaseInsensitive)) {
        fileName += "." + extension;
    }

    return QDir(settings.outputFolder).filePath(fileName);
}
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    rows_ = {};

    buildUi();
    renderSubtitleTable();
    applyStyle();

    subtitleTable_->selectRow(2);
    updateLinePropertiesFromRow(2);
}

void MainWindow::buildUi()
{
    setWindowTitle(kAppTitle);
    resize(1365, 768);

    auto* root = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    centralContainer_ = new QWidget(root);
    auto* workspaceLayout = new QVBoxLayout(centralContainer_);
    workspaceLayout->setContentsMargins(0, 0, 0, 0);
    workspaceLayout->setSpacing(0);

    buildTopBar();

    auto* splitter = new QSplitter(Qt::Horizontal, centralContainer_);
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(1);

    buildSubtitleTable();
    buildLinePropertiesPanel();

    splitter->addWidget(subtitleTable_);
    splitter->addWidget(startTimeEdit_->parentWidget()->parentWidget());
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setSizes({1050, kPropertiesPanelWidth});

    workspaceLayout->addWidget(splitter, 1);

    rootLayout->addWidget(centralContainer_, 1);
    setCentralWidget(root);

    buildStatusBar();
}

void MainWindow::buildTopBar()
{
    auto* topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(kTopBarHeight);

    auto* layout = new QHBoxLayout(topBar);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(12);

    auto* title = new QLabel(kAppTitle, topBar);
    title->setObjectName("appTitle");
    title->setAlignment(Qt::AlignVCenter);

    auto* menuBar = new QMenuBar(topBar);
    menuBar->setObjectName("topMenuBar");
    menuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto* fileMenu = menuBar->addMenu("File");
    auto *newAction = fileMenu->addAction("New");
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewSrtFile);
    auto* openFileAction = fileMenu->addAction("Open");
    connect(openFileAction, &QAction::triggered, this, &MainWindow::onOpenSrtFile);
    auto* saveFileAction = fileMenu->addAction("Save");
    connect(saveFileAction, &QAction::triggered, this, &MainWindow::onSaveSrtFile);
    auto* saveAsFileAction = fileMenu->addAction("Save as...");
    connect(saveAsFileAction, &QAction::triggered, this, &MainWindow::onSaveAsSrtFile);
    fileMenu->addSeparator();
    auto* closeAction = fileMenu->addAction("Close");
    connect(closeAction, &QAction::triggered, this, [this](){this->close();});

    auto* editMenu = menuBar->addMenu("Edit");
    auto *addLineAction = editMenu->addAction("Add line");
    connect(addLineAction, &QAction::triggered, this, &MainWindow::addLine);
    auto *delLineAction = editMenu->addAction("Delete line");
    connect(delLineAction, &QAction::triggered, this, &MainWindow::delLine);
    auto *undoAction = editMenu->addAction("Undo");
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);
    auto *redoAction = editMenu->addAction("Redo");
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);

    auto* toolsMenu = menuBar->addMenu("Tools");
    auto* ttsMenu = toolsMenu->addMenu("Text to speech");
    auto* ttsOption = ttsMenu->addAction("Options");
    connect(ttsOption, &QAction::triggered, this, &MainWindow::openTextToSpeechOption);
    auto* ttsConvert = ttsMenu->addAction("Convert");
    connect(ttsConvert, &QAction::triggered, this, &MainWindow::convertToSpeech);
    auto* ttsConvertAll = ttsMenu->addAction("Convert all");
    connect(ttsConvertAll, &QAction::triggered, this, &MainWindow::convertAllToSpeech);
    auto* helpMenu = menuBar->addMenu("Help");
    auto* aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, [this](){
        AboutWindow _aboutWindow(this);
        _aboutWindow.exec();
    });

    auto* titleMenuContainer = new QWidget(topBar);
    auto* titleMenuLayout = new QHBoxLayout(titleMenuContainer);
    titleMenuLayout->setContentsMargins(0, 0, 0, 0);
    titleMenuLayout->setSpacing(20);
    titleMenuLayout->addWidget(title, 0, Qt::AlignVCenter);
    titleMenuLayout->addWidget(menuBar, 0, Qt::AlignVCenter);

    layout->addWidget(titleMenuContainer, 0, Qt::AlignVCenter);

    layout->addStretch();

    auto* openButton = createToolbarButton("↗", "Open");
    connect(openButton, &QToolButton::clicked, this, &MainWindow::onOpenSrtFile);
    layout->addWidget(openButton);
    auto* saveButton = createToolbarButton("▣", "Save");
    connect(saveButton, &QToolButton::clicked, this, &MainWindow::onSaveSrtFile);
    layout->addWidget(saveButton);
    auto* undoButton = createToolbarButton("↶", "Undo");
    connect(undoButton, &QToolButton::clicked, this, &MainWindow::undo);
    layout->addWidget(undoButton);
    auto* redoButton = createToolbarButton("↷", "Redo");
    connect(redoButton, &QToolButton::clicked, this, &MainWindow::redo);
    layout->addWidget(redoButton);
    auto* addLineButton = createToolbarButton("⊞", "Add line");
    connect(addLineButton, &QToolButton::clicked, this, &MainWindow::addLine);
    layout->addWidget(addLineButton);
    auto* delButton = createToolbarButton("🗑", "Delete line");
    connect(delButton, &QToolButton::clicked, this, &MainWindow::delLine);
    layout->addWidget(delButton);

    qobject_cast<QVBoxLayout*>(centralContainer_->layout())->insertWidget(0, topBar);
}

QToolButton* MainWindow::createToolbarButton(const QString& text, const QString& tooltip)
{
    auto* button = new QToolButton(this);
    button->setObjectName("toolbarButton");
    button->setText(text);
    button->setToolTip(tooltip);
    button->setFixedSize(26, 26);
    return button;
}

QPushButton* MainWindow::createPanelButton(const QString& text, QWidget& parent)
{
    auto* button = new QPushButton(text, &parent);
    button->setObjectName("propertiesActionButton");
    button->setFixedHeight(40);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return button;
}

void MainWindow::buildSubtitleTable()
{
    subtitleTable_ = new QTableWidget(this);
    subtitleTable_->setObjectName("subtitleTable");
    subtitleTable_->setColumnCount(4);
    subtitleTable_->setHorizontalHeaderLabels({"#", "Start Time", "End Time", "Subtitle Text"});
    subtitleTable_->verticalHeader()->setVisible(false);
    subtitleTable_->setShowGrid(true);
    subtitleTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    subtitleTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    subtitleTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    subtitleTable_->setFocusPolicy(Qt::NoFocus);
    subtitleTable_->setAlternatingRowColors(true);

    subtitleTable_->horizontalHeader()->setSectionResizeMode(kColumnIndex, QHeaderView::Fixed);
    subtitleTable_->horizontalHeader()->setSectionResizeMode(kColumnStartTime, QHeaderView::Fixed);
    subtitleTable_->horizontalHeader()->setSectionResizeMode(kColumnEndTime, QHeaderView::Fixed);
    subtitleTable_->horizontalHeader()->setSectionResizeMode(kColumnSubtitleText, QHeaderView::Stretch);

    subtitleTable_->setColumnWidth(kColumnIndex, 55);
    subtitleTable_->setColumnWidth(kColumnStartTime, 105);
    subtitleTable_->setColumnWidth(kColumnEndTime, 105);

    connect(subtitleTable_, &QTableWidget::currentCellChanged, this, [this](int currentRow) {
        updateLinePropertiesFromRow(currentRow);
    });
}

void MainWindow::buildLinePropertiesPanel()
{
    auto* panel = new QWidget(this);
    panel->setObjectName("propertiesPanel");
    panel->setFixedWidth(kPropertiesPanelWidth);

    auto* rootLayout = new QVBoxLayout(panel);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto* header = new QWidget(panel);
    header->setObjectName("propertiesHeader");
    header->setFixedHeight(32);

    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(8, 0, 8, 0);

    auto* title = new QLabel("LINE PROPERTIES", header);
    title->setObjectName("panelTitle");
    headerLayout->addWidget(title);
    headerLayout->addStretch();

    auto* pin = new QLabel("⚑", header);
    pin->setObjectName("pinIcon");
    headerLayout->addWidget(pin);

    rootLayout->addWidget(header);

    auto* content = new QWidget(panel);
    auto* formLayout = new QVBoxLayout(content);
    formLayout->setContentsMargins(8, 10, 8, 0);
    formLayout->setSpacing(6);

    formLayout->addWidget(createPanelLabel("Start Time"));
    startTimeEdit_ = createTimeEditor("");
    formLayout->addWidget(startTimeEdit_);

    formLayout->addSpacing(8);
    formLayout->addWidget(createPanelLabel("End Time"));
    endTimeEdit_ = createTimeEditor("");
    formLayout->addWidget(endTimeEdit_);

    formLayout->addSpacing(8);
    formLayout->addWidget(createPanelLabel("Duration"));
    durationEdit_ = createTimeEditor("", true);
    formLayout->addWidget(durationEdit_);

    formLayout->addSpacing(16);
    formLayout->addWidget(createPanelLabel("Subtitle Text"));

    subtitleTextEdit_ = new QTextEdit(content);
    subtitleTextEdit_->setObjectName("subtitleTextEdit");
    subtitleTextEdit_->setFixedHeight(138);
    formLayout->addWidget(subtitleTextEdit_);

    formLayout->addSpacing(6);
    auto* convertToSpeechButton = createPanelButton("Convert to speech", *content);
    connect(convertToSpeechButton, &QPushButton::clicked, this, &MainWindow::convertToSpeech);
    formLayout->addWidget(convertToSpeechButton);

    formLayout->addStretch();
    rootLayout->addWidget(content, 1);

    connect(startTimeEdit_, &QLineEdit::editingFinished, this, &MainWindow::updateCurrentRowFromLineProperties);
    connect(endTimeEdit_, &QLineEdit::editingFinished, this, &MainWindow::updateCurrentRowFromLineProperties);
    connect(subtitleTextEdit_, &QTextEdit::textChanged, this, &MainWindow::updateCurrentRowFromLineProperties);
}

QLabel* MainWindow::createPanelLabel(const QString& text)
{
    auto* label = new QLabel(text, this);
    label->setObjectName("formLabel");
    return label;
}

QLineEdit* MainWindow::createTimeEditor(const QString& text, bool readOnly)
{
    auto* editor = new QLineEdit(this);
    editor->setObjectName(readOnly ? "readonlyTimeEditor" : "timeEditor");
    editor->setText(text);
    editor->setReadOnly(readOnly);
    editor->setFixedHeight(30);
    return editor;
}

void MainWindow::buildStatusBar()
{
    auto* bar = statusBar();
    bar->setObjectName("statusBar");
    bar->setFixedHeight(kStatusBarHeight);
    bar->setSizeGripEnabled(false);

    statusLeftLabel_ = new QLabel("Ready!", bar);
    statusRightLabel_ = new QLabel("truonghaidang.com", bar);

    bar->addWidget(statusLeftLabel_, 1);
    bar->addPermanentWidget(statusRightLabel_);
}

void MainWindow::renderSubtitleTable()
{
    subtitleTable_->clearContents();
    subtitleTable_->setRowCount(rows_.size());

    for (int row = 0; row < rows_.size(); ++row) {
        const SubtitleRow& subtitle = rows_.at(row);

        auto* indexItem = new QTableWidgetItem(QString::number(subtitle.index));
        auto* startItem = new QTableWidgetItem(subtitle.startTime);
        auto* endItem = new QTableWidgetItem(subtitle.endTime);
        auto* textItem = new QTableWidgetItem(subtitle.text);

        indexItem->setTextAlignment(Qt::AlignCenter);
        startItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        endItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        subtitleTable_->setItem(row, kColumnIndex, indexItem);
        subtitleTable_->setItem(row, kColumnStartTime, startItem);
        subtitleTable_->setItem(row, kColumnEndTime, endItem);
        subtitleTable_->setItem(row, kColumnSubtitleText, textItem);
        subtitleTable_->setRowHeight(row, kRowHeight);
    }
}

void MainWindow::updateLinePropertiesFromRow(int row)
{
    if (row < 0 || row >= rows_.size()) {
        return;
    }

    const SubtitleRow& subtitle = rows_.at(row);
    startTimeEdit_->setText(subtitle.startTime);
    endTimeEdit_->setText(subtitle.endTime);
    durationEdit_->setText(subtitle.duration);
    subtitleTextEdit_->setPlainText(subtitle.text);
}

void MainWindow::handleSelectedRowChanged()
{
    updateLinePropertiesFromRow(subtitleTable_->currentRow());
}

void MainWindow::applyStyle()
{
    QFile file(":/css/mainwindow.qss");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    this->setStyleSheet(stream.readAll());
}

QString MainWindow::calculateDuration(const QString& startTime, const QString& endTime)
{
    const QTime start = QTime::fromString(startTime, "HH:mm:ss,zzz");
    const QTime end = QTime::fromString(endTime, "HH:mm:ss,zzz");

    if (!start.isValid() || !end.isValid()) {
        return "00:00:00,000";
    }

    int durationMs = start.msecsTo(end);

    if (durationMs < 0) {
        durationMs += 24 * 60 * 60 * 1000;
    }

    const int hours = durationMs / 3600000;
    durationMs %= 3600000;

    const int minutes = durationMs / 60000;
    durationMs %= 60000;

    const int seconds = durationMs / 1000;
    const int milliseconds = durationMs % 1000;

    return QString("%1:%2:%3,%4")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(milliseconds, 3, 10, QChar('0'));
}

void MainWindow::onNewSrtFile()
{
    this->_currentSrtFilePath.clear();
    this->rows_.clear();
    this->subtitleTable_->clear();
    this->startTimeEdit_->clear();
    this->endTimeEdit_->clear();
    this->durationEdit_->clear();
    this->subtitleTextEdit_->clear();
}

void MainWindow::onOpenSrtFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Subtitle File",
        QString(),
        "Subtitle Files (*.srt *.vtt *.txt);;All Files (*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    this->_currentSrtFilePath = filePath;

    this->rows_.clear();

    while (!stream.atEnd()) {

        // Index
        QString indexLine = stream.readLine().trimmed();

        if (indexLine.isEmpty()) {
            continue;
        }

        // Time
        QString timeLine = stream.readLine().trimmed();

        QStringList timeParts = timeLine.split(" --> ");

        if (timeParts.size() != 2) {
            continue;
        }

        QString startTime = timeParts.at(0).trimmed();
        QString endTime = timeParts.at(1).trimmed();

        // Subtitle text
        QString subtitleText;

        while (!stream.atEnd()) {

            QString line = stream.readLine();

            if (line.trimmed().isEmpty()) {
                break;
            }

            if (!subtitleText.isEmpty()) {
                subtitleText += "\n";
            }

            subtitleText += line;
        }

        SubtitleRow row;

        row.index = indexLine.toInt();
        row.startTime = startTime;
        row.endTime = endTime;
        row.duration = calculateDuration(startTime, endTime);
        row.text = subtitleText;

        this->rows_.append(row);
    }

    renderSubtitleTable();

    if (!this->rows_.isEmpty()) {
        subtitleTable_->selectRow(0);
        updateLinePropertiesFromRow(0);
    }
    this->statusLeftLabel_->setText(QString("Opened file %1").arg(filePath));
}

void MainWindow::onSaveSrtFile()
{
    QString filePath = this->_currentSrtFilePath;

    if (filePath.isEmpty()) {

        filePath = QFileDialog::getSaveFileName(
            this,
            "Save Subtitle File",
            QString(),
            "SRT Files (*.srt);;All Files (*)"
        );

        if (filePath.isEmpty()) {
            return;
        }
    }

    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    for (const SubtitleRow& row : this->rows_) {

        stream << row.index << "\n";

        stream << row.startTime
               << " --> "
               << row.endTime
               << "\n";

        stream << row.text << "\n\n";
    }

    file.close();

    this->_currentSrtFilePath = filePath;
    this->statusLeftLabel_->setText(QString("File is saved at %1").arg(filePath));
}

void MainWindow::onSaveAsSrtFile()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Subtitle File As",
        this->_currentSrtFilePath,
        "SRT Files (*.srt);;All Files (*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    for (const SubtitleRow& row : this->rows_) {

        stream << row.index << "\n";

        stream << row.startTime
               << " --> "
               << row.endTime
               << "\n";

        stream << row.text << "\n\n";
    }

    file.close();

    this->_currentSrtFilePath = filePath;
    this->statusLeftLabel_->setText(QString("New file is saved at %1").arg(filePath));
}

void MainWindow::updateCurrentRowFromLineProperties()
{
    const int rowIndex = subtitleTable_->currentRow();

    if (rowIndex < 0 || rowIndex >= rows_.size()) {
        return;
    }

    SubtitleRow& row = rows_[rowIndex];

    row.startTime = startTimeEdit_->text().trimmed();
    row.endTime = endTimeEdit_->text().trimmed();
    row.duration = calculateDuration(row.startTime, row.endTime);
    row.text = subtitleTextEdit_->toPlainText();

    if (subtitleTable_->item(rowIndex, kColumnStartTime)) {
        subtitleTable_->item(rowIndex, kColumnStartTime)->setText(row.startTime);
    }
    subtitleTable_->item(rowIndex, kColumnEndTime)->setText(row.endTime);
    subtitleTable_->item(rowIndex, kColumnSubtitleText)->setText(row.text);

    durationEdit_->setText(row.duration);
}

void MainWindow::addLine()
{
    saveUndoState();
    SubtitleRow row;

    row.index = this->rows_.size() + 1;
    row.startTime = "00:00:00,000";
    row.endTime = "00:00:00,000";
    row.duration = calculateDuration(row.startTime, row.endTime);
    row.text = "";

    this->rows_.append(row);

    this->renderSubtitleTable();

    const int newRowIndex = this->rows_.size() - 1;

    this->subtitleTable_->selectRow(newRowIndex);
    this->updateLinePropertiesFromRow(newRowIndex);

    this->statusLeftLabel_->setText(
        QString("Added line %1").arg(row.index)
    );
}

void MainWindow::delLine()
{
    if (this->rows_.isEmpty()) {
        return;
    }

    saveUndoState();

    int rowIndex = subtitleTable_->currentRow();

    // Không có dòng nào được chọn
    if (rowIndex < 0 || rowIndex >= this->rows_.size()) {
        rowIndex = this->rows_.size() - 1;
    }

    this->rows_.removeAt(rowIndex);

    // Re-index lại subtitle
    for (int i = 0; i < this->rows_.size(); ++i) {
        this->rows_[i].index = i + 1;
    }

    renderSubtitleTable();

    // Không còn dòng nào
    if (this->rows_.isEmpty()) {

        startTimeEdit_->clear();
        endTimeEdit_->clear();
        durationEdit_->clear();
        subtitleTextEdit_->clear();

        return;
    }

    // Select lại dòng hợp lệ
    if (rowIndex >= this->rows_.size()) {
        rowIndex = this->rows_.size() - 1;
    }

    subtitleTable_->selectRow(rowIndex);
    updateLinePropertiesFromRow(rowIndex);
}

void MainWindow::saveUndoState()
{
    undoStack_.append(this->rows_);
    redoStack_.clear();
}

void MainWindow::restoreRows(const QVector<SubtitleRow>& rows)
{
    this->rows_ = rows;

    this->renderSubtitleTable();

    if (this->rows_.isEmpty()) {
        this->startTimeEdit_->clear();
        this->endTimeEdit_->clear();
        this->durationEdit_->clear();
        this->subtitleTextEdit_->clear();
        return;
    }

    this->subtitleTable_->selectRow(0);
    this->updateLinePropertiesFromRow(0);
}

void MainWindow::undo()
{
    if (undoStack_.isEmpty()) {
        return;
    }

    redoStack_.append(this->rows_);

    const QVector<SubtitleRow> previousState = undoStack_.takeLast();
    restoreRows(previousState);

    this->statusLeftLabel_->setText("Undo");
}

void MainWindow::redo()
{
    if (redoStack_.isEmpty()) {
        return;
    }

    undoStack_.append(this->rows_);

    const QVector<SubtitleRow> nextState = redoStack_.takeLast();
    restoreRows(nextState);

    this->statusLeftLabel_->setText("Redo");
}

void MainWindow::openTextToSpeechOption()
{
    TTSWindow ttsWindow(this);
    ttsWindow.exec();
}

void MainWindow::convertToSpeech()
{
    const int currentRow = subtitleTable_->currentRow();
    if (currentRow < 0 || currentRow >= rows_.size()) {
        QMessageBox::warning(this, "Text to speech", "Please select a subtitle line to convert.");
        return;
    }

    convertRowsToSpeech({currentRow});
}

void MainWindow::convertAllToSpeech()
{
    if (rows_.isEmpty()) {
        QMessageBox::warning(this, "Text to speech", "There are no subtitle lines to convert.");
        return;
    }

    QVector<int> rowIndexes;
    rowIndexes.reserve(rows_.size());

    for (int rowIndex = 0; rowIndex < rows_.size(); ++rowIndex) {
        rowIndexes.append(rowIndex);
    }

    convertRowsToSpeech(rowIndexes);
}

void MainWindow::convertRowsToSpeech(const QVector<int>& rowIndexes)
{
    if (rowIndexes.isEmpty()) {
        return;
    }

    updateCurrentRowFromLineProperties();

    TextToSpeechSettings settings = readTextToSpeechSettings();
    if (settings.apiKey.isEmpty() || settings.voiceId.isEmpty() || settings.outputFolder.isEmpty()) {
        QMessageBox::warning(
            this,
            "Text to speech",
            "Please configure the ElevenLabs API key, voice, and output folder before converting. Use Load Voices, select a voice, then save settings."
        );

        TTSWindow ttsWindow(this);
        if (ttsWindow.exec() != QDialog::Accepted) {
            return;
        }

        settings = readTextToSpeechSettings();
        if (settings.apiKey.isEmpty() || settings.voiceId.isEmpty() || settings.outputFolder.isEmpty()) {
            QMessageBox::warning(
                this,
                "Text to speech",
                "The ElevenLabs API key, voice ID, and output folder are required."
            );
            return;
        }
    }

    QDir outputDirectory(settings.outputFolder);
    if (!outputDirectory.exists() && !outputDirectory.mkpath(".")) {
        QMessageBox::critical(
            this,
            "Text to speech",
            QString("Cannot create output folder: %1").arg(settings.outputFolder)
        );
        return;
    }

    ElevenLabsClient client(settings.apiKey.toStdString());
    int convertedCount = 0;
    int skippedCount = 0;

    for (int index = 0; index < rowIndexes.size(); ++index) {
        const int rowIndex = rowIndexes.at(index);
        if (rowIndex < 0 || rowIndex >= rows_.size()) {
            ++skippedCount;
            continue;
        }

        const SubtitleRow& subtitle = rows_.at(rowIndex);
        const QString text = subtitle.text.trimmed();
        if (text.isEmpty()) {
            ++skippedCount;
            continue;
        }

        if (text.size() > settings.maxCharsPerRequest) {
            QMessageBox::warning(
                this,
                "Text to speech",
                QString("Line %1 is longer than the configured Max Chars/Req.").arg(subtitle.index)
            );
            ++skippedCount;
            continue;
        }

        const QString outputPath = buildTextToSpeechOutputPath(
            settings,
            _currentSrtFilePath,
            subtitle.index,
            text
        );

        statusLeftLabel_->setText(
            QString("Converting line %1 to speech...").arg(subtitle.index)
        );
        QApplication::processEvents();

        try {
            const ElevenLabsTextToSpeechRequest request = createTextToSpeechRequest(settings, text);
            const ElevenLabsResponse response = client.textToSpeech(request);

            if (response.status_code < 200 || response.status_code >= 300) {
                const QString errorBody = QString::fromUtf8(
                    response.body.data(),
                    static_cast<qsizetype>(response.body.size())
                ).left(500);
                QMessageBox::warning(
                    this,
                    "Text to speech",
                    QString("Convert line %1 failed. HTTP status: %2\n%3")
                        .arg(subtitle.index)
                        .arg(response.status_code)
                        .arg(errorBody)
                );
                ++skippedCount;
                continue;
            }

            if (response.body.empty()) {
                QMessageBox::warning(
                    this,
                    "Text to speech",
                    QString("Convert line %1 failed because ElevenLabs returned empty audio.").arg(subtitle.index)
                );
                ++skippedCount;
                continue;
            }

            QFile outputFile(outputPath);
            if (!outputFile.open(QIODevice::WriteOnly)) {
                QMessageBox::critical(
                    this,
                    "Text to speech",
                    QString("Cannot write audio file: %1").arg(outputPath)
                );
                ++skippedCount;
                continue;
            }

            const qint64 bytesWritten = outputFile.write(
                response.body.data(),
                static_cast<qint64>(response.body.size())
            );
            if (bytesWritten != static_cast<qint64>(response.body.size())) {
                QMessageBox::critical(
                    this,
                    "Text to speech",
                    QString("Cannot write complete audio file: %1").arg(outputPath)
                );
                ++skippedCount;
                continue;
            }

            ++convertedCount;
        } catch (const std::exception& error) {
            QMessageBox::critical(
                this,
                "Text to speech",
                QString("Convert line %1 failed: %2")
                    .arg(subtitle.index)
                    .arg(QString::fromUtf8(error.what()))
            );
            ++skippedCount;
            continue;
        }

        if (settings.delayMs > 0 && index < rowIndexes.size() - 1) {
            QThread::msleep(static_cast<unsigned long>(settings.delayMs));
        }
    }

    statusLeftLabel_->setText(
        QString("Text to speech complete. Converted %1, skipped %2.")
            .arg(convertedCount)
            .arg(skippedCount)
    );

    QMessageBox::information(
        this,
        "Text to speech",
        QString("Converted %1 subtitle line(s). Skipped %2.").arg(convertedCount).arg(skippedCount)
    );
}

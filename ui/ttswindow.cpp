#include "ttswindow.h"

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

const char* kWindowTitle = "Text To Speech Settings";
const char* kDefaultApiKeyMask = "************************";
const char* kDefaultOutputFolder = "C:\\Users\\Admin\\Documents\\TTS_Output";
const char* kDefaultFilePattern = "{project}_{voice}_{date}";
const char* kDefaultPreviewText =
    "The quick brown fox jumps over the lazy dog. This is a sample text to test the current voice configuration.";
}

TTSWindow::TTSWindow(QWidget* parent)
    : QDialog(parent)
{
    buildUi();
    applyStyle();
}

bool TTSWindow::eventFilter(QObject* watched, QEvent* event)
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

void TTSWindow::buildUi()
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

void TTSWindow::buildTitleBar(QVBoxLayout& rootLayout)
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
    connect(closeButton, &QPushButton::clicked, this, &TTSWindow::reject);
    titleLayout->addWidget(closeButton);

    rootLayout.addWidget(titleBar_);
}

void TTSWindow::buildContent(QVBoxLayout& rootLayout)
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

void TTSWindow::buildSidebar(QWidget& parent, QHBoxLayout& contentLayout)
{
    auto* sidebar = new QListWidget(&parent);
    sidebar->setObjectName("ttsSidebar");
    sidebar->setFixedWidth(kSidebarWidth);
    sidebar->setFocusPolicy(Qt::NoFocus);
    sidebar->addItem("ElevenLabs API");
    sidebar->setCurrentRow(0);

    contentLayout.addWidget(sidebar);
}

void TTSWindow::buildSettingsPanel(QWidget& parent, QHBoxLayout& contentLayout)
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

    auto* formLayout = new QGridLayout();
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setHorizontalSpacing(16);
    formLayout->setVerticalSpacing(12);
    formLayout->setColumnMinimumWidth(0, kFormLabelWidth);
    formLayout->setColumnStretch(1, 1);

    int row = 0;
    addElevenLabsSection(*formLayout, *panel, row);
    addVoiceOverridesSection(*formLayout, *panel, row);
    addProcessingSection(*formLayout, *panel, row);
    addPreviewSection(*formLayout, *panel, row);

    panelLayout->addLayout(formLayout);
    panelLayout->addStretch();

    scrollArea->setWidget(panel);
    contentLayout.addWidget(scrollArea, 1);
}

void TTSWindow::buildActionBar(QVBoxLayout& rootLayout)
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

    connect(saveButton, &QPushButton::clicked, this, &TTSWindow::accept);
    connect(cancelButton, &QPushButton::clicked, this, &TTSWindow::reject);
    connect(applyButton, &QPushButton::clicked, this, []() {});

    actionLayout->addWidget(saveButton);
    actionLayout->addWidget(cancelButton);
    actionLayout->addWidget(applyButton);

    rootLayout.addWidget(actionBar);
}

void TTSWindow::applyStyle()
{
    QFile file(":/css/ttswindow.qss");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    setStyleSheet(stream.readAll());
}

void TTSWindow::addElevenLabsSection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    auto* title = createSectionTitle("ElevenLabs API Settings", parent);
    formLayout.addWidget(title, row, 0, 1, 2);
    ++row;

    auto* topSeparator = createSeparator(parent);
    formLayout.addWidget(topSeparator, row, 0, 1, 2);
    ++row;

    auto* apiKeyEdit = createLineEdit(kDefaultApiKeyMask, parent, QLineEdit::Password);
    auto* testButton = createButton("Test Connection", parent);
    auto* apiKeyRow = new QWidget(&parent);
    auto* apiKeyLayout = new QHBoxLayout(apiKeyRow);
    apiKeyLayout->setContentsMargins(0, 0, 0, 0);
    apiKeyLayout->setSpacing(8);
    apiKeyLayout->addWidget(apiKeyEdit, 1);
    apiKeyLayout->addWidget(testButton);
    formLayout.addWidget(createFormLabel("API Key:", parent), row, 0);
    formLayout.addWidget(apiKeyRow, row, 1);
    ++row;

    auto* modelComboBox = createComboBox({
        "eleven_multilingual_v2",
        "eleven_turbo_v2",
        "eleven_monolingual_v1",
    }, parent);
    formLayout.addWidget(createFormLabel("Model:", parent), row, 0);
    formLayout.addWidget(modelComboBox, row, 1);
    ++row;

    auto* voiceComboBox = createComboBox({
        "Rachel (American, Narration)",
        "Drew (American, News)",
        "Clyde (American, Conversational)",
    }, parent);
    auto* loadVoicesButton = createButton("Load Voices", parent);
    auto* voiceRow = new QWidget(&parent);
    auto* voiceLayout = new QHBoxLayout(voiceRow);
    voiceLayout->setContentsMargins(0, 0, 0, 0);
    voiceLayout->setSpacing(8);
    voiceLayout->addWidget(voiceComboBox, 1);
    voiceLayout->addWidget(loadVoicesButton);
    formLayout.addWidget(createFormLabel("Voice:", parent), row, 0);
    formLayout.addWidget(voiceRow, row, 1);
    ++row;

    auto* outputFormatComboBox = createComboBox({
        "mp3_44100_128",
        "mp3_44100_192",
        "pcm_44100",
        "ulaw_8000",
    }, parent);
    formLayout.addWidget(createFormLabel("Output Format:", parent), row, 0);
    formLayout.addWidget(outputFormatComboBox, row, 1);
    ++row;
}

void TTSWindow::addVoiceOverridesSection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    formLayout.addWidget(createSeparator(parent), row, 0, 1, 2);
    ++row;

    formLayout.addWidget(createSectionTitle("Voice Overrides", parent), row, 0, 1, 2);
    ++row;

    formLayout.addWidget(createFormLabel("Stability:", parent), row, 0);
    formLayout.addWidget(createSliderRow("stabilitySlider", 50, parent), row, 1);
    ++row;

    formLayout.addWidget(createFormLabel("Similarity:", parent), row, 0);
    formLayout.addWidget(createSliderRow("similaritySlider", 75, parent), row, 1);
    ++row;

    formLayout.addWidget(createFormLabel("Style:", parent), row, 0);
    formLayout.addWidget(createSliderRow("styleSlider", 0, parent), row, 1);
    ++row;

    auto* speakerBoostCheckBox = new QCheckBox("Speaker Boost", &parent);
    speakerBoostCheckBox->setObjectName("speakerBoostCheckBox");
    speakerBoostCheckBox->setChecked(true);
    formLayout.addWidget(speakerBoostCheckBox, row, 1);
    ++row;
}

void TTSWindow::addProcessingSection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    formLayout.addWidget(createSeparator(parent), row, 0, 1, 2);
    ++row;

    formLayout.addWidget(createSectionTitle("Processing & Output", parent), row, 0, 1, 2);
    ++row;

    outputFolderEdit_ = createLineEdit(kDefaultOutputFolder, parent);
    auto* browseButton = createButton("Browse...", parent);
    connect(browseButton, &QPushButton::clicked, this, &TTSWindow::browseOutputFolder);

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
    formLayout.addWidget(createLineEdit(kDefaultFilePattern, parent), row, 1);
    ++row;

    auto* limitsRow = new QWidget(&parent);
    auto* limitsLayout = new QHBoxLayout(limitsRow);
    limitsLayout->setContentsMargins(0, 0, 0, 0);
    limitsLayout->setSpacing(16);

    auto* maxCharsEdit = createLineEdit("5000", parent);
    maxCharsEdit->setFixedWidth(kSmallInputWidth);
    auto* delayLabel = new QLabel("Delay (ms):", limitsRow);
    auto* delayEdit = createLineEdit("250", parent);
    delayEdit->setFixedWidth(kDelayInputWidth);

    limitsLayout->addWidget(maxCharsEdit);
    limitsLayout->addWidget(delayLabel);
    limitsLayout->addWidget(delayEdit);
    limitsLayout->addStretch();

    formLayout.addWidget(createFormLabel("Max Chars/Req:", parent), row, 0);
    formLayout.addWidget(limitsRow, row, 1);
    ++row;
}

void TTSWindow::addPreviewSection(QGridLayout& formLayout, QWidget& parent, int& row)
{
    formLayout.addWidget(createSeparator(parent), row, 0, 1, 2);
    ++row;

    auto* previewTextEdit = new QTextEdit(&parent);
    previewTextEdit->setObjectName("previewTextEdit");
    previewTextEdit->setFixedHeight(kPreviewHeight);
    previewTextEdit->setPlainText(kDefaultPreviewText);

    auto* playPreviewButton = createButton("▷  Play Preview", parent);
    auto* previewContent = new QWidget(&parent);
    auto* previewLayout = new QVBoxLayout(previewContent);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->setSpacing(8);
    previewLayout->addWidget(previewTextEdit);
    previewLayout->addWidget(playPreviewButton, 0, Qt::AlignRight);

    formLayout.addWidget(createFormLabel("Preview:", parent), row, 0, Qt::AlignTop);
    formLayout.addWidget(previewContent, row, 1);
    ++row;
}

QLabel* TTSWindow::createFormLabel(const QString& text, QWidget& parent) const
{
    auto* label = new QLabel(text, &parent);
    label->setObjectName("ttsFormLabel");
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return label;
}

QLabel* TTSWindow::createSectionTitle(const QString& text, QWidget& parent) const
{
    auto* label = new QLabel(text, &parent);
    label->setObjectName("ttsSectionTitle");
    return label;
}

QFrame* TTSWindow::createSeparator(QWidget& parent) const
{
    auto* line = new QFrame(&parent);
    line->setObjectName("ttsSeparator");
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    return line;
}

QLineEdit* TTSWindow::createLineEdit(const QString& text, QWidget& parent, QLineEdit::EchoMode echoMode) const
{
    auto* edit = new QLineEdit(text, &parent);
    edit->setObjectName("ttsLineEdit");
    edit->setEchoMode(echoMode);
    edit->setMinimumHeight(30);
    return edit;
}

QComboBox* TTSWindow::createComboBox(const QStringList& values, QWidget& parent) const
{
    auto* comboBox = new QComboBox(&parent);
    comboBox->setObjectName("ttsComboBox");
    comboBox->addItems(values);
    comboBox->setMinimumHeight(30);
    return comboBox;
}

QWidget* TTSWindow::createSliderRow(const QString& objectName, int initialValue, QWidget& parent) const
{
    auto* container = new QWidget(&parent);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto* slider = new QSlider(Qt::Horizontal, container);
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

QPushButton* TTSWindow::createButton(const QString& text, QWidget& parent) const
{
    auto* button = new QPushButton(text, &parent);
    button->setObjectName("ttsButton");
    button->setMinimumHeight(kButtonHeight);
    button->setMinimumWidth(kButtonWidth);
    return button;
}

void TTSWindow::browseOutputFolder()
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

#include "mainwindow.h"

namespace
{
constexpr int kTopBarHeight = 48;
constexpr int kStatusBarHeight = 24;
constexpr int kPropertiesPanelWidth = 270;
constexpr int kRowHeight = 28;

constexpr int kColumnIndex = 0;
constexpr int kColumnStartTime = 1;
constexpr int kColumnEndTime = 2;
constexpr int kColumnSubtitleText = 3;

const char* kAppTitle = "SubtitleEdit Pro";
const char* kSelectedRowColor = "#d3e3ff";
const char* kBorderColor = "#c0c0c0";
const char* kFrameBackground = "#f0f0f0";
const char* kWorkspaceBackground = "#ffffff";
const char* kHeaderBackground = "#e5e5e5";
const char* kTextColor = "#1a1c1c";
const char* kSecondaryTextColor = "#404752";
const char* kPrimaryColor = "#0078d4";
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    rows_ = {
        {1, "00:00:05,000", "00:00:08,500", "00:00:03,500", "Hello, world! Welcome to SubtitleEdit Pro."},
        {2, "00:00:09,100", "00:00:12,000", "00:00:02,900", "This is a highly professional, native-feeling application."},
        {3, "00:00:12,500", "00:00:15,800", "00:00:03,300", "It leverages Qt6 inspired design principles."},
        {4, "00:00:16,000", "00:00:20,000", "00:00:04,000", "Information density and clear affordances are prioritized."}
    };

    buildUi();
    loadSampleData();
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
    layout->addWidget(title);

    QStringList menus = {"File", "Edit", "Tools", "Help"};
    for (const QString& menuText : menus) {
        auto* button = new QPushButton(menuText, topBar);
        button->setObjectName("menuButton");
        button->setFlat(true);
        layout->addWidget(button);
    }

    layout->addStretch();

    layout->addWidget(createToolbarButton("↗", "Open"));
    layout->addWidget(createToolbarButton("▣", "Save"));
    layout->addWidget(createToolbarButton("↶", "Undo"));
    layout->addWidget(createToolbarButton("↷", "Redo"));
    layout->addWidget(createToolbarButton("⊞", "Add line"));
    layout->addWidget(createToolbarButton("🗑", "Delete line"));

    qobject_cast<QVBoxLayout*>(centralContainer_->layout())->insertWidget(0, topBar);
}

QWidget* MainWindow::createToolbarButton(const QString& text, const QString& tooltip)
{
    auto* button = new QToolButton(this);
    button->setObjectName("toolbarButton");
    button->setText(text);
    button->setToolTip(tooltip);
    button->setFixedSize(26, 26);
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
    startTimeEdit_ = createTimeEditor("00:00:12,500");
    formLayout->addWidget(startTimeEdit_);

    formLayout->addSpacing(8);
    formLayout->addWidget(createPanelLabel("End Time"));
    endTimeEdit_ = createTimeEditor("00:00:15,800");
    formLayout->addWidget(endTimeEdit_);

    formLayout->addSpacing(8);
    formLayout->addWidget(createPanelLabel("Duration"));
    durationEdit_ = createTimeEditor("00:00:03,300", true);
    formLayout->addWidget(durationEdit_);

    formLayout->addSpacing(16);
    formLayout->addWidget(createPanelLabel("Subtitle Text"));

    subtitleTextEdit_ = new QTextEdit(content);
    subtitleTextEdit_->setObjectName("subtitleTextEdit");
    subtitleTextEdit_->setFixedHeight(138);
    subtitleTextEdit_->setPlainText("It leverages Qt6 inspired design principles.");
    formLayout->addWidget(subtitleTextEdit_);

    formLayout->addStretch();
    rootLayout->addWidget(content, 1);
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

    statusLeftLabel_ = new QLabel("v1.2.4 | UTF-8 | LINE: 124, COL: 12", bar);
    statusRightLabel_ = new QLabel("DOCUMENTATION    REPORT BUG", bar);

    bar->addWidget(statusLeftLabel_, 1);
    bar->addPermanentWidget(statusRightLabel_);
}

void MainWindow::loadSampleData()
{
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
    qApp->setStyleSheet(QString(R"(
        QMainWindow {
            background: %1;
            color: %2;
            font-family: Inter, Arial, sans-serif;
            font-size: 13px;
        }

        QWidget#topBar {
            background: %1;
            border-bottom: 1px solid %3;
        }

        QLabel#appTitle {
            font-size: 12px;
            font-weight: 700;
            color: #202020;
            padding-left: 8px;
            padding-right: 8px;
        }

        QPushButton#menuButton {
            border: none;
            background: transparent;
            color: #303030;
            padding: 4px 8px;
            font-size: 12px;
        }

        QPushButton#menuButton:hover,
        QToolButton#toolbarButton:hover {
            background: #e5e5e5;
        }

        QToolButton#toolbarButton {
            border: none;
            background: transparent;
            color: #303030;
            font-size: 15px;
        }

        QTableWidget#subtitleTable {
            background: %4;
            alternate-background-color: #f9f9f9;
            gridline-color: #eeeeee;
            border: none;
            color: %2;
            font-size: 13px;
            selection-background-color: %5;
            selection-color: #001c39;
        }

        QTableWidget#subtitleTable::item {
            padding-left: 8px;
            border-right: 1px solid #eeeeee;
        }

        QHeaderView::section {
            background: %1;
            color: %6;
            border: none;
            border-right: 1px solid %3;
            border-bottom: 1px solid %3;
            padding: 5px 8px;
            font-size: 12px;
            font-weight: 600;
        }

        QWidget#propertiesPanel {
            background: %1;
            border-left: 1px solid %3;
        }

        QWidget#propertiesHeader {
            background: %7;
            border-bottom: 1px solid %3;
        }

        QLabel#panelTitle {
            color: %6;
            font-size: 12px;
            font-weight: 700;
            letter-spacing: 1px;
        }

        QLabel#pinIcon {
            color: #202020;
            font-size: 13px;
        }

        QLabel#formLabel {
            color: %6;
            font-size: 12px;
        }

        QLineEdit#timeEditor,
        QTextEdit#subtitleTextEdit {
            background: #ffffff;
            border: 1px solid %3;
            color: %2;
            padding: 6px 8px;
            font-size: 13px;
        }

        QLineEdit#timeEditor:focus,
        QTextEdit#subtitleTextEdit:focus {
            border: 1px solid %8;
        }

        QLineEdit#readonlyTimeEditor {
            background: #e8e8e8;
            border: 1px solid %3;
            color: #b8b8b8;
            padding: 6px 8px;
            font-size: 13px;
        }

        QStatusBar#statusBar {
            background: %1;
            border-top: 1px solid %3;
            color: #5f6670;
            font-size: 11px;
            text-transform: uppercase;
        }

        QStatusBar::item {
            border: none;
        }
    )")
    .arg(kFrameBackground)
    .arg(kTextColor)
    .arg(kBorderColor)
    .arg(kWorkspaceBackground)
    .arg(kSelectedRowColor)
    .arg(kSecondaryTextColor)
    .arg(kHeaderBackground)
    .arg(kPrimaryColor));
}
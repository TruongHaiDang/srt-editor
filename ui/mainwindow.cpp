#include "mainwindow.h"
#include <qaction.h>
#include <qdebug.h>
#include <qfiledevice.h>
#include <qfiledialog.h>
#include <qmainwindow.h>
#include <qobject.h>
#include <qpicture.h>
#include <qstringconverter_base.h>

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

const char* kAppTitle = "SubtitleEdit Free";
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
    layout->addWidget(title);

    auto* menuBar = new QMenuBar(topBar);
    menuBar->setObjectName("topMenuBar");

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

    menuBar->addMenu("Tools");
    menuBar->addMenu("Help");

    layout->addWidget(menuBar);

    layout->addStretch();

    auto* openButton = createToolbarButton("↗", "Open");
    connect(openButton, &QToolButton::clicked, this, &MainWindow::onOpenSrtFile);
    layout->addWidget(openButton);
    auto* saveButton = createToolbarButton("▣", "Save");
    connect(saveButton, &QToolButton::clicked, this, &MainWindow::onSaveSrtFile);
    layout->addWidget(saveButton);
    layout->addWidget(createToolbarButton("↶", "Undo"));
    layout->addWidget(createToolbarButton("↷", "Redo"));
    layout->addWidget(createToolbarButton("⊞", "Add line"));
    layout->addWidget(createToolbarButton("🗑", "Delete line"));

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

    statusLeftLabel_ = new QLabel("v1.2.4 | UTF-8 | LINE: 124, COL: 12", bar);
    statusRightLabel_ = new QLabel("DOCUMENTATION    REPORT BUG", bar);

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
    qApp->setStyleSheet(stream.readAll());
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

void MainWindow::undo()
{

}

void MainWindow::redo()
{

}


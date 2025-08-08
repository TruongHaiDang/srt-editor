#include "mainwindow.h"
#include <QRegularExpression>
#include <QSettings>

/**
 * @brief Construct a new MainWindow object.
 *
 * Problem/Objectives: Initialize the application interface, load language
 * list, configure signal–slot connections and prepare an empty subtitle list.
 *
 * Solution/Implementation: Build designer UI, customise icons, dynamically
 * populate the languages menu from languages.txt and connect every QAction
 * with its corresponding private slot.
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // Thiết lập UI
    ui->setupUi(this);
    setWindowIcon(QIcon(":/mainwindow/assets/icon.png"));

    subtitleContainerLayout->setObjectName("subtitleContainerLayout");
    ui->subtitleContainer->setLayout(subtitleContainerLayout);

    connect(ui->actionClose, &QAction::triggered, this, &QApplication::quit);
    connect(ui->actionSoftware_Author, &QAction::triggered, this, &MainWindow::openAbout);
    connect(ui->actionAdd_subtitle, &QAction::triggered, this, &MainWindow::addSubtitle);
    connect(ui->actionClear_subtitles, &QAction::triggered, this, &MainWindow::clearSubtitles);
    connect(ui->actionRemove_subtitle, &QAction::triggered, this, &MainWindow::removeSubtitle);
    connect(ui->actionOutput_folder, &QAction::triggered, this, &MainWindow::selectOutputSpeechDir);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openSrtFile);
    connect(ui->actionExport, &QAction::triggered, this, &MainWindow::exportSrtFile);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::openSettings);
    connect(ui->actionTranslate_all, &QAction::triggered, this, &MainWindow::translateAll);
    connect(ui->actionTranslate_selected, &QAction::triggered, this, &MainWindow::translateSelected);
    connect(ui->actionUpdate_end_time, &QAction::triggered, this, &MainWindow::updateEndTime);
    connect(ui->actionCurrent_subtitle, &QAction::triggered, this, &MainWindow::currentSubtitleTextToSpeech);
    connect(ui->actionView_audio_files, &QAction::triggered, this, &MainWindow::openOutputFolder);

    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    statusBar()->addPermanentWidget(progressBar);
    
    statusBar()->showMessage("Ready");
}

/**
 * @brief Destructor – deletes the generated Ui object.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Display the modal "About" dialog that shows application information.
 */
void MainWindow::openAbout()
{
    QDialog *aboutDlg = new QDialog(this);
    Ui::About *about_ui = new Ui::About();
    about_ui->setupUi(aboutDlg);

    std::string versionStr = "v" + std::string(PROJECT_VERSION);
    about_ui->projectNameLbl->setText(QString::fromStdString(std::string(PROJECT_NAME)));
    about_ui->versionLbl->setText(QString::fromStdString(versionStr));
    connect(about_ui->openYoutubeBtn, &QPushButton::clicked, this, []()
            { QDesktopServices::openUrl(QUrl("https://www.youtube.com/channel/UCsqybGJRpU6XBo7-D9cRYkg/")); });

    aboutDlg->exec();

    delete about_ui;
    delete aboutDlg;
}

/**
 * @brief Create a new SubtitleItem, append it to internal list and layout.
 */
void MainWindow::addSubtitle()
{
    SubtitleItem *newSubtitle = new SubtitleItem();
    int latestOrder = this->subtitles.length();
    newSubtitle->setOrder(latestOrder);
    connect(newSubtitle->getSelectedCheckbox(), &QCheckBox::stateChanged, this, &MainWindow::appendOrRemoveSelectedSubtitle);
    subtitleContainerLayout->addWidget(newSubtitle);
    subtitles.append(newSubtitle);
    statusBar()->showMessage("New subtitle is added.");
}

/**
 * @brief Remove all widgets and sub-layouts within the given layout pointer.
 */
void MainWindow::clearLayout(QLayout *layout)
{
    if (!layout)
        return;

    while (QLayoutItem *item = layout->takeAt(0))
    {
        if (QLayout *childLayout = item->layout())
        {
            clearLayout(childLayout);
            delete childLayout;
        };

        if (QWidget *widget = item->widget())
        {
            widget->hide();
            widget->deleteLater();
        };

        delete item;
    }
}

/**
 * @brief Clear the entire subtitle list and update the status bar.
 */
void MainWindow::clearSubtitles()
{
    subtitles.clear();
    this->clearLayout(subtitleContainerLayout);
    statusBar()->showMessage("All subtitles are cleared.");
}

/**
 * @brief Delete user-selected subtitle rows and renumber remaining ones.
 */
void MainWindow::removeSubtitle()
{
    // Xóa các subtitle được chọn khỏi layout và danh sách subtitles
    for (SubtitleItem *item : selectedSubtitles)
    {
        subtitles.removeAll(item);
        subtitleContainerLayout->removeWidget(item);
        item->deleteLater();
    }
    selectedSubtitles.clear();

    // Cập nhật lại order cho các subtitle còn lại
    for (int i = 0; i < subtitles.size(); ++i)
    {
        subtitles[i]->setOrder(i);
    }

    statusBar()->showMessage("Removed selected subtitles.");
}

/**
 * @brief Let user pick an output directory for synthesized speech files.
 */
void MainWindow::selectOutputSpeechDir()
{
    this->outputSpeechDir = QFileDialog::getExistingDirectory(
        this,
        "Select the output folder for speech mp3 files",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks).toStdString();
    
    statusBar()->showMessage("Output dir: " + QString::fromStdString(this->outputSpeechDir));
}

/**
 * @brief Maintain the selectedSubtitles cache whenever a checkbox toggles.
 */
void MainWindow::appendOrRemoveSelectedSubtitle(int state)
{
    QCheckBox *checkbox = qobject_cast<QCheckBox *>(sender());
    if (!checkbox)
        return;

    SubtitleItem *item = qobject_cast<SubtitleItem *>(checkbox->parentWidget());
    if (!item)
        return;

    QUuid uuid = item->getUuid();

    if (state == Qt::Checked)
    {
        // Thêm vào danh sách nếu chưa có
        bool exists = false;
        for (SubtitleItem *s : selectedSubtitles)
        {
            if (s->getUuid() == uuid)
            {
                exists = true;
                break;
            }
        }
        if (!exists)
            selectedSubtitles.append(item);
    }
    else if (state == Qt::Unchecked)
    {
        // Xóa khỏi danh sách nếu có
        for (int i = 0; i < selectedSubtitles.size(); ++i)
        {
            if (selectedSubtitles[i]->getUuid() == uuid)
            {
                selectedSubtitles.removeAt(i);
                break;
            }
        }
    }
}

/**
 * @brief Parse an existing SRT file and populate the editor with its blocks.
 */
void MainWindow::openSrtFile()
{
    progressBar->setVisible(true);
    progressBar->setValue(0);
    statusBar()->showMessage("Loading subtitles...");

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open SRT File",
        QDir::homePath(),
        "SubRip Subtitle (*.srt);;All Files (*)"
    );
    if (filePath.isEmpty()) return;

    this->srtFilePath = filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        statusBar()->showMessage("Cannot open file: " + filePath);
        return;
    }

    QTextStream in(&file);
    QString srtContent = in.readAll();
    file.close();

    // Xóa các subtitle hiện tại
    subtitles.clear();
    clearLayout(subtitleContainerLayout);

    // Phân tích từng block subtitle
    QStringList blocks = srtContent.split(QRegularExpression("\n\s*\n"), Qt::SkipEmptyParts);
    int order = 0;
    int totalBlocks = blocks.size();
    progressBar->setMaximum(totalBlocks);
    
    for (int i = 0; i < blocks.size(); ++i) {
        const QString &block = blocks[i];
        QStringList lines = block.split('\n', Qt::SkipEmptyParts);
        if (lines.size() < 3) {
            progressBar->setValue(i + 1);
            continue;
        }

        QString timeLine = lines[1].trimmed();
        QRegularExpression timeRx("([0-9]{2}):([0-9]{2}):([0-9]{2}),([0-9]{3}) --> ([0-9]{2}):([0-9]{2}):([0-9]{2}),([0-9]{3})");
        QRegularExpressionMatch match = timeRx.match(timeLine);
        if (!match.hasMatch()) {
            progressBar->setValue(i + 1);
            continue;
        }

        SubtitleItem *item = new SubtitleItem();
        item->setOrder(order++);
        item->setStartHour(match.captured(1).toInt());
        item->setStartMinute(match.captured(2).toInt());
        item->setStartSecond(match.captured(3).toInt());
        item->setStartMillisecond(match.captured(4).toInt());
        item->setEndHour(match.captured(5).toInt());
        item->setEndMinute(match.captured(6).toInt());
        item->setEndSecond(match.captured(7).toInt());
        item->setEndMillisecond(match.captured(8).toInt());

        QString content;
        for (int j = 2; j < lines.size(); ++j) {
            if (j > 2) content += "\n";
            content += lines[j];
        }
        item->setContent(content);

        connect(item->getSelectedCheckbox(), &QCheckBox::stateChanged, this, &MainWindow::appendOrRemoveSelectedSubtitle);
        subtitleContainerLayout->addWidget(item);
        subtitles.append(item);

        // Cập nhật progress bar cho mỗi item được xử lý
        progressBar->setValue(i + 1);
        
        // Cho phép UI update mỗi 5 items để tránh lag quá nhiều
        if (i % 5 == 0) {
            QCoreApplication::processEvents();
        }
    }

    progressBar->setVisible(false);
    statusBar()->showMessage("Opened: " + filePath);
}

/**
 * @brief Validate every SubtitleItem then write them into a user-chosen SRT output file.
 */
void MainWindow::exportSrtFile()
{
    if (subtitles.isEmpty()) {
        statusBar()->showMessage("No subtitles to export.");
        return;
    }

    ExportValidator validator;
    int prevEndMs = -1;
    for (int i = 0; i < subtitles.size(); ++i) {
        SubtitleItem* item = subtitles[i];
        if (!validator.timestampValidator(item)) {
            statusBar()->showMessage(QString("Subtitle #%1 có thời gian không hợp lệ!").arg(i + 1));
            return;
        }
        // Kiểm tra định dạng dấu phẩy
        QString timeLine = QString("%1:%2:%3,%4 --> %5:%6:%7,%8")
            .arg(item->getStartHour(), 2, 10, QChar('0'))
            .arg(item->getStartMinute(), 2, 10, QChar('0'))
            .arg(item->getStartSecond(), 2, 10, QChar('0'))
            .arg(item->getStartMillisecond(), 3, 10, QChar('0'))
            .arg(item->getEndHour(), 2, 10, QChar('0'))
            .arg(item->getEndMinute(), 2, 10, QChar('0'))
            .arg(item->getEndSecond(), 2, 10, QChar('0'))
            .arg(item->getEndMillisecond(), 3, 10, QChar('0'));
        if (!validator.timestampFormatValid(timeLine)) {
            statusBar()->showMessage(QString("Subtitle #%1 sai định dạng dấu phẩy cho milliseconds!").arg(i + 1));
            return;
        }
        // Kiểm tra chồng thời gian
        int startMs = ((item->getStartHour() * 60 * 60 + item->getStartMinute() * 60 + item->getStartSecond()) * 1000) + item->getStartMillisecond();
        if (prevEndMs != -1 && startMs < prevEndMs) {
            statusBar()->showMessage(QString("Subtitle #%1 bị chồng thời gian với subtitle #%2!").arg(i + 1).arg(i));
            return;
        }
        prevEndMs = ((item->getEndHour() * 60 * 60 + item->getEndMinute() * 60 + item->getEndSecond()) * 1000) + item->getEndMillisecond();
    }

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export SRT File",
        QDir::homePath(),
        "SubRip Subtitle (*.srt);;All Files (*)"
    );
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        statusBar()->showMessage("Cannot write file: " + filePath);
        return;
    }

    QTextStream out(&file);
    for (int i = 0; i < subtitles.size(); ++i) {
        SubtitleItem* item = subtitles[i];
        out << QString::number(i + 1) << "\n";
        out << QString("%1:%2:%3,%4 --> %5:%6:%7,%8\n")
            .arg(item->getStartHour(), 2, 10, QChar('0'))
            .arg(item->getStartMinute(), 2, 10, QChar('0'))
            .arg(item->getStartSecond(), 2, 10, QChar('0'))
            .arg(item->getStartMillisecond(), 3, 10, QChar('0'))
            .arg(item->getEndHour(), 2, 10, QChar('0'))
            .arg(item->getEndMinute(), 2, 10, QChar('0'))
            .arg(item->getEndSecond(), 2, 10, QChar('0'))
            .arg(item->getEndMillisecond(), 3, 10, QChar('0'));
        out << item->getContent() << "\n\n";
    }
    file.close();
    statusBar()->showMessage("Exported: " + filePath);
}

void MainWindow::openSettings()
{
    AppSettings settingsDlg(this);
    if (settingsDlg.exec() == QDialog::Accepted) {
        loadAppSettings();
    }
}

void MainWindow::loadAppSettings()
{
    QSettings settings;
    QString translateProvider = settings.value("translate/provider", "OpenAI").toString();
    QString translateApiKey = settings.value("translate/apiKey", "").toString();
    QString ttsProvider = settings.value("tts/provider", "OpenAI").toString();
    QString ttsApiKey = settings.value("tts/apiKey", "").toString();
}

void MainWindow::translateAll()
{
    for (SubtitleItem *item: subtitles)
    {
        item->translate();
    }
    statusBar()->showMessage("Translation for all subtitles is completed.");
}

void MainWindow::translateSelected()
{
    SubtitleItem *item = selectedSubtitles.last();
    item->translate();
    statusBar()->showMessage("Translation for the latest selected subtitle is completed.");
}

void MainWindow::allSubtitlesTextToSpeech()
{
    for (SubtitleItem *item: selectedSubtitles)
    {
        item->textToSpeech(this->outputSpeechDir);
    }
    statusBar()->showMessage("Text to speech for selected subtitles is completed.");
}

void MainWindow::currentSubtitleTextToSpeech()
{
    SubtitleItem *item = selectedSubtitles.last();
    item->textToSpeech(this->outputSpeechDir);
    statusBar()->showMessage("Text to speech for the latest selected subtitle is completed.");
}

void MainWindow::updateEndTime()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open SRT File",
        QDir::homePath(),
        "SubRip Subtitle (*.srt);;All Files (*)"
    );
    if (filePath.isEmpty()) return;

    if (this->selectedSubtitles.length() > 0)
    {
        TextToSpeech tts;
        TextToSpeech::AudioTime audioTime = tts.getAudioLength(filePath.toStdString());
        this->selectedSubtitles.last()->setEndHour(audioTime.hours);
        this->selectedSubtitles.last()->setEndMinute(audioTime.minutes);
        this->selectedSubtitles.last()->setEndSecond(audioTime.seconds);
        this->selectedSubtitles.last()->setEndMillisecond(audioTime.milliseconds);
    };

    this->ui->statusbar->showMessage("Set end time for the latest selected subtitle.");
}

void MainWindow::openOutputFolder()
{
    if (!this->outputSpeechDir.empty())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(this->outputSpeechDir)));
    }
}

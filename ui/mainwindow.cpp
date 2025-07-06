#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // Thiết lập UI
    ui->setupUi(this);
    setWindowIcon(QIcon(":/mainwindow/assets/icon.png"));

    subtitleContainerLayout->setObjectName("subtitleContainerLayout");
    ui->subtitleContainer->setLayout(subtitleContainerLayout);

    std::ifstream file("languages.txt");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            QAction *action = new QAction(line.c_str(), this);
            line[0] = std::toupper(line[0]);
            std::string actionObjName = "action" + line;
            action->setObjectName(actionObjName);
            action->setData(QString::fromStdString(line));
            connect(action, &QAction::triggered, this, &MainWindow::setLanguage);
            ui->menuLanguages->addAction(action);
        }
    };

    connect(ui->actionClose, &QAction::triggered, this, &QApplication::quit);
    connect(ui->actionSoftware_Author, &QAction::triggered, this, &MainWindow::openAbout);
    connect(ui->actionAdd_subtitle, &QAction::triggered, this, &MainWindow::addSubtitle);
    connect(ui->actionClear_subtitles, &QAction::triggered, this, &MainWindow::clearSubtitles);
    connect(ui->actionRemove_subtitle, &QAction::triggered, this, &MainWindow::removeSubtitle);
    connect(ui->actionOutput_folder, &QAction::triggered, this, &MainWindow::selectOutputSpeechDir);

    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow()
{
    delete ui;
}

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

void MainWindow::clearSubtitles()
{
    subtitles.clear();
    this->clearLayout(subtitleContainerLayout);
    statusBar()->showMessage("All subtitles are cleared.");
}

void MainWindow::removeSubtitle()
{
    // Xóa các subtitle được chọn khỏi layout và danh sách subtitles
    for (SubtitleItem* item : selectedSubtitles) {
        subtitles.removeAll(item);
        subtitleContainerLayout->removeWidget(item);
        item->deleteLater();
    }
    selectedSubtitles.clear();

    // Cập nhật lại order cho các subtitle còn lại
    for (int i = 0; i < subtitles.size(); ++i) {
        subtitles[i]->setOrder(i);
    }

    statusBar()->showMessage("Removed selected subtitles.");
}

void MainWindow::selectOutputSpeechDir()
{
    this->outputSpeechDir = QFileDialog::getExistingDirectory(
        this,
        "Select the output folder for speech mp3 files",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    statusBar()->showMessage("Output dir: " + this->outputSpeechDir);
}

void MainWindow::setLanguage()
{
    QAction *action = qobject_cast<QAction*>(this->sender());
    if (!action) return;
    selectedLanguage = action->data().toString();
    statusBar()->showMessage("Selected language: " + selectedLanguage);
}

void MainWindow::appendOrRemoveSelectedSubtitle(int state)
{
    QCheckBox *checkbox = qobject_cast<QCheckBox*>(sender());
    if (!checkbox) return;

    SubtitleItem *item = qobject_cast<SubtitleItem*>(checkbox->parentWidget());
    if (!item) return;

    QUuid uuid = item->getUuid();

    if (state == Qt::Checked)
    {
        // Thêm vào danh sách nếu chưa có
        bool exists = false;
        for (SubtitleItem* s : selectedSubtitles) {
            if (s->getUuid() == uuid) {
                exists = true;
                break;
            }
        }
        if (!exists) selectedSubtitles.append(item);
    }
    else if (state == Qt::Unchecked)
    {
        // Xóa khỏi danh sách nếu có
        for (int i = 0; i < selectedSubtitles.size(); ++i) {
            if (selectedSubtitles[i]->getUuid() == uuid) {
                selectedSubtitles.removeAt(i);
                break;
            }
        }
    }
}

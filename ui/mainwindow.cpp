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
            ui->menuLanguages->addAction(action);
        }
    };

    connect(ui->actionClose, &QAction::triggered, this, &QApplication::quit);
    connect(ui->actionSoftware_Author, &QAction::triggered, this, &MainWindow::openAbout);
    connect(ui->actionAdd_subtitle, &QAction::triggered, this, &MainWindow::addSubtitle);
    connect(ui->actionClear_subtitles, &QAction::triggered, this, &MainWindow::clearSubtitles);
    connect(ui->actionRemove_subtitle, &QAction::triggered, this, &MainWindow::removeSubtitle);

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
}

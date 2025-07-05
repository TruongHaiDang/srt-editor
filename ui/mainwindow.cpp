#include "mainwindow.h"

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
    connect(about_ui->openYoutubeBtn, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://www.youtube.com/channel/UCsqybGJRpU6XBo7-D9cRYkg/"));
    });

    aboutDlg->exec();

    delete about_ui;
    delete aboutDlg;
}

void MainWindow::addSubtitle()
{
    SubtitleItem *newSubtitle = new SubtitleItem();
    int latestOrder = this->subtitles.length();
    printf("%d", latestOrder);
    newSubtitle->setOrder(latestOrder);
    subtitleContainerLayout->addWidget(newSubtitle);
    subtitles.append(newSubtitle);
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_about.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // Thiết lập UI
    ui->setupUi(this);
    setWindowIcon(QIcon(":/mainwindow/assets/icon.png"));

    connect(ui->actionSoftware_Author, &QAction::triggered, this, &MainWindow::openAbout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openAbout()
{
    QDialog aboutDlg(this);
    Ui::About about_ui;
    about_ui.setupUi(&aboutDlg);
    aboutDlg.exec();
}

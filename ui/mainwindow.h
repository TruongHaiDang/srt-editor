#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QMainWindow>
#include <QWidget>
#include <QIcon>
#include <QDialog>

namespace Ui {
    class MainWindow;
    class About;
}

class MainWindow: public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

    void openAbout();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

#endif
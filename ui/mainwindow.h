#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "ui_mainwindow.h"
#include "ui_about.h"
#include "subtitle_item.h"
#include "configure.h"
#include <stdio.h>

#include <string>
#include <QString>
#include <QMainWindow>
#include <QWidget>
#include <QIcon>
#include <QDialog>
#include <QDesktopServices>
#include <QList>
#include <QVBoxLayout>

namespace Ui {
    class MainWindow;
    class About;
}

class MainWindow: public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    QVBoxLayout *subtitleContainerLayout = new QVBoxLayout();
    QList<SubtitleItem*> subtitles;

    void openAbout();
    void addSubtitle();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

#endif
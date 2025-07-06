#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "ui_mainwindow.h"
#include "ui_about.h"
#include "subtitle_item.h"
#include "configure.h"
#include <sstream>
#include <fstream>

#include <string>
#include <QString>
#include <QMainWindow>
#include <QWidget>
#include <QIcon>
#include <QDialog>
#include <QDesktopServices>
#include <QList>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QLayout>
#include <QLayoutItem>
#include <QFileDialog>
#include <QString>
#include <QDir>

namespace Ui {
    class MainWindow;
    class About;
}

class MainWindow: public QMainWindow
{
    Q_OBJECT

private:
    QString outputSpeechDir;
    QString selectedLanguage;
    Ui::MainWindow *ui;
    QVBoxLayout *subtitleContainerLayout = new QVBoxLayout();
    QList<SubtitleItem*> subtitles;

    void clearLayout(QLayout *layout);
    void openAbout();
    void addSubtitle();
    void removeSubtitle();
    void clearSubtitles();
    void selectOutputSpeechDir();
    void setLanguage();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

#endif
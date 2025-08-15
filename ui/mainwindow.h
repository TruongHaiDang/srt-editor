#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "ui_mainwindow.h"
#include "ui_about.h"
#include "subtitle_item.h"
#include "app_settings.h"
#include "configure.h"
#include <sstream>
#include <fstream>
#include "ExportValidator.h"

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
#include <QUuid>
#include <QProgressBar>

namespace Ui {
    class MainWindow;
    class About;
}

/**
 * @class MainWindow
 * @brief The primary window that orchestrates subtitle editing workflow.
 *
 * MainWindow wires UI actions (menu, buttons) to business logic such as adding
 * or exporting subtitles. It owns a dynamic list of SubtitleItem widgets and
 * bundles common helper methods that manipulate the list.
 */
class MainWindow: public QMainWindow
{
    Q_OBJECT

private:
    std::string outputSpeechDir;
    Ui::MainWindow *ui;
    QVBoxLayout *subtitleContainerLayout = new QVBoxLayout();
    QList<SubtitleItem*> subtitles;
    QList<SubtitleItem*> selectedSubtitles;
    QString srtFilePath;
    QProgressBar *progressBar = nullptr;

    /**
     * @brief Recursively delete all items inside a layout.
     *
     * Problem/Objectives: When clearing the subtitle list we must delete
     * widgets AND any nested layouts to prevent memory leaks.
     *
     * Solution/Implementation: Iterate through layout items and depending on
     * their type delete them appropriately.
     */
    void clearLayout(QLayout *layout);
    /**
     * @brief Show the "About" dialog.
     */
    void openAbout();
    /**
     * @brief Append a fresh SubtitleItem to the list.
     */
    void addSubtitle();
    /**
     * @brief Remove all currently selected subtitle rows.
     */
    void removeSubtitle();
    /**
     * @brief Delete every subtitle row and reset internal state.
     */
    void clearSubtitles();
    /**
     * @brief Prompt the user to select an output directory for generated audio files.
     */
    void selectOutputSpeechDir();
    /**
     * @brief Add or remove a SubtitleItem from the selection cache whenever its checkbox changes.
     */
    void appendOrRemoveSelectedSubtitle(int state);
    /**
     * @brief Ask user for an existing SRT file and populate the editor list with its contents.
     */
    void openSrtFile();
    /**
     * @brief Validate all subtitles and export them to a new SRT file.
     */
    void exportSrtFile();

    void openSettings();

    void loadAppSettings();

    void translateAll();
    
    void translateSelected();

    void updateEndTime();

    void currentSubtitleTextToSpeech();

    void allSubtitlesTextToSpeech();

    void openOutputFolder();

    void saveSubtitlesWithoutValidator();

public:
    /**
     * @brief Default constructor – builds the UI and connects signals.
     */
    explicit MainWindow(QWidget *parent = nullptr);
    /**
     * @brief Destructor – cleans up allocated Ui class.
     */
    ~MainWindow();
};

#endif
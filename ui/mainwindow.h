#pragma once

#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QFile>
#include <QtWidgets/QMenu>
#include <qobject.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTime>

#include "aboutwindow.h"
#include "ttswindow.h"

class QAction;
class QLabel;
class QLineEdit;
class QTextEdit;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void handleSelectedRowChanged();

private:
    struct SubtitleRow
    {
        int index;
        QString startTime;
        QString endTime;
        QString duration;
        QString text;
    };

    void buildUi();
    void buildTopBar();
    void buildSubtitleTable();
    void buildLinePropertiesPanel();
    void buildStatusBar();

    QToolButton* createToolbarButton(const QString& text, const QString& tooltip);
    QLabel* createPanelLabel(const QString& text);
    QLineEdit* createTimeEditor(const QString& text, bool readOnly = false);
    QString calculateDuration(const QString& startTime, const QString& endTime);

    void renderSubtitleTable();
    void applyStyle();
    void updateLinePropertiesFromRow(int row);
    void updateCurrentRowFromLineProperties();
    void saveUndoState();
    void restoreRows(const QVector<SubtitleRow>& rows);

    void onNewSrtFile();
    void onOpenSrtFile();
    void onSaveSrtFile();
    void onSaveAsSrtFile();

    void addLine();
    void delLine();
    void undo();
    void redo();

    void openTextToSpeechOption();
    void convertToSpeech();
    void convertAllToSpeech();

private:
    QWidget* centralContainer_ = nullptr;
    QTableWidget* subtitleTable_ = nullptr;

    QLineEdit* startTimeEdit_ = nullptr;
    QLineEdit* endTimeEdit_ = nullptr;
    QLineEdit* durationEdit_ = nullptr;
    QTextEdit* subtitleTextEdit_ = nullptr;

    QLabel* statusLeftLabel_ = nullptr;
    QLabel* statusRightLabel_ = nullptr;

    QVector<SubtitleRow> rows_;
    QString _currentSrtFilePath;
    QVector<QVector<SubtitleRow>> undoStack_;
    QVector<QVector<SubtitleRow>> redoStack_;
};

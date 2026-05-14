#pragma once

#include <QtCore/QPoint>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSlider>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

class QComboBox;
class QEvent;
class QFrame;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QListWidget;
class QPushButton;
class QSlider;
class QTextEdit;
class QVBoxLayout;
class QWidget;

/**
 * Text-to-speech settings dialog.
 *
 * Input: optional parent widget used for modal ownership.
 * Output: QDialog::Accepted when the user saves, QDialog::Rejected when cancelled.
 * Errors: this UI layer does not perform network/file writes; invalid settings are not persisted here.
 * Assumption: backend TTS integration will read validated settings from this dialog in a later step.
 */
class TTSWindow final : public QDialog
{
    Q_OBJECT

public:
    explicit TTSWindow(QWidget* parent = nullptr);
    ~TTSWindow() override = default;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void buildUi();
    void buildTitleBar(QVBoxLayout& rootLayout);
    void buildContent(QVBoxLayout& rootLayout);
    void buildSidebar(QWidget& parent, QHBoxLayout& contentLayout);
    void buildSettingsPanel(QWidget& parent, QHBoxLayout& contentLayout);
    void buildActionBar(QVBoxLayout& rootLayout);
    void applyStyle();

    void addElevenLabsSection(QGridLayout& formLayout, QWidget& parent, int& row);
    void addVoiceOverridesSection(QGridLayout& formLayout, QWidget& parent, int& row);
    void addProcessingSection(QGridLayout& formLayout, QWidget& parent, int& row);
    void addPreviewSection(QGridLayout& formLayout, QWidget& parent, int& row);

    QLabel* createFormLabel(const QString& text, QWidget& parent) const;
    QLabel* createSectionTitle(const QString& text, QWidget& parent) const;
    QFrame* createSeparator(QWidget& parent) const;
    QLineEdit* createLineEdit(const QString& text, QWidget& parent, QLineEdit::EchoMode echoMode = QLineEdit::Normal) const;
    QComboBox* createComboBox(const QStringList& values, QWidget& parent) const;
    QWidget* createSliderRow(const QString& objectName, int initialValue, QWidget& parent) const;
    QPushButton* createButton(const QString& text, QWidget& parent) const;

    void browseOutputFolder();

    QWidget* titleBar_ = nullptr;
    QLineEdit* outputFolderEdit_ = nullptr;
    QPoint dragPosition_;
};

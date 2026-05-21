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
#include <QtWidgets/QStackedWidget>
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
class QStackedWidget;
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
class SettingWindow final : public QDialog
{
    Q_OBJECT

public:
    explicit SettingWindow(QWidget* parent = nullptr);
    ~SettingWindow() override = default;

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
    void loadSettings();
    void saveSettings() const;

    void addElevenLabsSection(QGridLayout& formLayout, QWidget& parent, int& row);
    void addOpenAISection(QGridLayout& formLayout, QWidget& parent, int& row);
    void addVoiceOverridesSection(QGridLayout& formLayout, QWidget& parent, int& row);
    void addProcessingSection(QGridLayout& formLayout, QWidget& parent, int& row);
    void addPreviewSection(QGridLayout& formLayout, QWidget& parent, int& row);
    QScrollArea* createSettingsScrollPage(QWidget& parent, QGridLayout*& formLayout) const;

    QLabel* createFormLabel(const QString& text, QWidget& parent) const;
    QLabel* createSectionTitle(const QString& text, QWidget& parent) const;
    QFrame* createSeparator(QWidget& parent) const;
    QLineEdit* createLineEdit(const QString& text, QWidget& parent, QLineEdit::EchoMode echoMode = QLineEdit::Normal) const;
    QComboBox* createComboBox(const QStringList& values, QWidget& parent) const;
    QWidget* createSliderRow(const QString& objectName, int initialValue, QWidget& parent, QSlider*& slider) const;
    QPushButton* createButton(const QString& text, QWidget& parent) const;
    QString createFilePatternToolTip() const;

    void saveAndAccept();
    void restoreComboBoxValue(QComboBox& comboBox, const QString& value) const;
    void browseOutputFolder();
    void testElevenLabsConnection();
    void loadElevenLabsVoices();

    QWidget* titleBar_ = nullptr;
    QStackedWidget* settingsStack_ = nullptr;
    QLineEdit* apiKeyEdit_ = nullptr;
    QLineEdit* openAiApiKeyEdit_ = nullptr;
    QComboBox* modelComboBox_ = nullptr;
    QComboBox* voiceComboBox_ = nullptr;
    QComboBox* outputFormatComboBox_ = nullptr;
    QSlider* stabilitySlider_ = nullptr;
    QSlider* similaritySlider_ = nullptr;
    QSlider* styleSlider_ = nullptr;
    QCheckBox* speakerBoostCheckBox_ = nullptr;
    QLineEdit* outputFolderEdit_ = nullptr;
    QLineEdit* filePatternEdit_ = nullptr;
    QLineEdit* maxCharsEdit_ = nullptr;
    QLineEdit* delayEdit_ = nullptr;
    QTextEdit* previewTextEdit_ = nullptr;
    QPoint dragPosition_;
};

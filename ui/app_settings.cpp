#include "app_settings.h"

AppSettings::AppSettings(QWidget *parent) : QDialog(parent), ui(new Ui::AppSettings)
{
    ui->setupUi(this);
    
    QSettings settings("haidanghth910", "srteditor");
    ui->translateAiProviders->setCurrentText(settings.value("translate/provider", "OpenAI").toString());
    ui->translateApiKey->setText(settings.value("translate/apiKey", "").toString());
    ui->ttsAiProvider->setCurrentText(settings.value("tts/provider", "OpenAI").toString());
    ui->ttsApiKey->setText(settings.value("tts/apiKey", "").toString());
}

AppSettings::~AppSettings()
{
}

void AppSettings::accept()
{
    QSettings settings("haidanghth910", "srteditor");
    settings.setValue("translate/provider", getTranslateProvider());
    settings.setValue("translate/apiKey", getTranslateApiKey());
    settings.setValue("tts/provider", getTtsProvider());
    settings.setValue("tts/apiKey", getTtsApiKey());
    QDialog::accept();
}

QString AppSettings::getTranslateProvider() const {
    return ui->translateAiProviders->currentText();
}
QString AppSettings::getTranslateApiKey() const {
    return ui->translateApiKey->text();
}
QString AppSettings::getTtsProvider() const {
    return ui->ttsAiProvider->currentText();
}
QString AppSettings::getTtsApiKey() const {
    return ui->ttsApiKey->text();
}

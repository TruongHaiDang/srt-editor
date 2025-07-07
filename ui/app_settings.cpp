#include "app_settings.h"
#include <QSettings>

AppSettings::AppSettings(QWidget *parent) : QDialog(parent), ui(new Ui::AppSettings)
{
    ui->setupUi(this);
}

AppSettings::~AppSettings()
{
}

void AppSettings::accept()
{
    QSettings settings;
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

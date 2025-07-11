#pragma once

#ifndef __APP_SETTINGS_H__
#define __APP_SETTINGS_H__

namespace Ui {
    class AppSettings;
}

#include <QDialog>
#include <QWidget>
#include "ui_app_settings.h"
#include <QSettings>

class AppSettings: public QDialog
{
    Q_OBJECT

private:
    Ui::AppSettings *ui;

    QString getTranslateProvider() const;
    QString getTranslateApiKey() const;
    QString getTtsProvider() const;
    QString getTtsApiKey() const;

public:
    explicit AppSettings(QWidget* parent = nullptr);
    ~AppSettings();

protected:
    void accept() override;
};

#endif

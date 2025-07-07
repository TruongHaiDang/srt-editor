#pragma once

#ifndef __APP_SETTINGS_H__
#define __APP_SETTINGS_H__

namespace Ui {
    class AppSettings;
}

#include <QDialog>
#include <QWidget>
#include "ui_app_settings.h"

class AppSettings: public QDialog
{
    Q_OBJECT

private:
    Ui::AppSettings *ui;

public:
    explicit AppSettings(QWidget* parent = nullptr);
    ~AppSettings();

    QString getTranslateProvider() const;
    QString getTranslateApiKey() const;
    QString getTtsProvider() const;
    QString getTtsApiKey() const;

protected:
    void accept() override;
};

#endif

#pragma once

#ifndef __SUBTITLE_CONFIG_H__
#define __SUBTITLE_CONFIG_H__

#include <QWidget>
#include <QDialog>
#include "ui_subtitle_config.h"

namespace Ui {
    class SubtitleConfig;
}

class SubtitleConfig: public QDialog
{
    Q_OBJECT

private:
    Ui::SubtitleConfig *ui;

public:
    explicit SubtitleConfig(QWidget* parent = nullptr);
    ~SubtitleConfig();
};

#endif

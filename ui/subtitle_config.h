#pragma once

#ifndef __SUBTITLE_CONFIG_H__
#define __SUBTITLE_CONFIG_H__

#include <QWidget>
#include <QDialog>
#include "ui_subtitle_config.h"
#include <QList>
#include <QString>

namespace Ui {
    class SubtitleConfig;
}

class SubtitleConfig: public QDialog
{
    Q_OBJECT

private:
    Ui::SubtitleConfig *ui;
    QList<QString> openaiChatModels;
    QList<QString> openaiSpeechModels;
    QList<QString> githubChatModels;
    QList<QString> elevenlabsSpeechModels;

    void getOpenaiChatModels();
    void getOpenaiSpeechModels();
    void getOpenaiSpeechVoices();

    void getElevenlabsSpeechModels();
    void getElevenlabsSpeechVoices();

    void getGithubChatModels();

public:
    explicit SubtitleConfig(QWidget* parent = nullptr);
    ~SubtitleConfig();
};

#endif

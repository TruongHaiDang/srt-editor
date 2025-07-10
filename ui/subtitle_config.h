#pragma once

#ifndef __SUBTITLE_CONFIG_H__
#define __SUBTITLE_CONFIG_H__

#include <QWidget>
#include <QDialog>
#include "ui_subtitle_config.h"
#include <QList>
#include <QString>
#include <QSettings>
#include <curl/curl.h>
#include <string>
#include <nlohmann/json.hpp>
#include <QList>
#include <QDebug>

namespace Ui {
    class SubtitleConfig;
}

class SubtitleConfig: public QDialog
{
    Q_OBJECT

private:
    Ui::SubtitleConfig *ui;
    std::string githubToken;
    std::string openaiApiKey;
    QList<QString> openaiChatModels;
    QList<QString> openaiSpeechModels;
    QList<QString> githubChatModels;
    QList<QString> elevenlabsSpeechModels;

    size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *userData);

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

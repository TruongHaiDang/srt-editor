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
#include <QSettings>
#include <QMap>
#include <QVariant>
#include <map>

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
    std::string elevenlabsApiKey;
    QList<QString> openaiChatModels;
    QList<QString> openaiSpeechModels;
    const QList<QString> openaiSpeechVoices {
        "alloy",
        "ash",
        "ballad",
        "coral",
        "echo",
        "fable",
        "nova",
        "onyx",
        "sage",
        "shimmer"
    };
    QList<QString> githubChatModels;
    QList<QString> elevenlabsSpeechModels;
    QList<QString> elevenlabsSpeechVoices;
    QMap<QString, QString> elevenlabsVoiceNameToId;

    static size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *userData);

    void getOpenaiModels();

    void getElevenlabsSpeechModels();
    void getElevenlabsSpeechVoices();

    void getGithubChatModels();

protected:
    void showEvent(QShowEvent *event) override;

public:
    explicit SubtitleConfig(QWidget* parent = nullptr);
    ~SubtitleConfig();

    std::map<QString, QVariant> getConfigs();

    /**
     * Lưu config cho một subtitle item theo id vào QSettings group.
     * @param itemId: Định danh subtitle item (ví dụ: số thứ tự dòng)
     * @param configs: Map chứa các config cần lưu
     */
    void saveConfigForSubtitleItem(int itemId, const std::map<QString, QVariant>& configs);

    /**
     * Đọc config cho một subtitle item theo id từ QSettings group.
     * @param itemId: Định danh subtitle item
     * @return: Map chứa các config đã lưu
     */
    std::map<QString, QVariant> loadConfigForSubtitleItem(int itemId);
};

#endif

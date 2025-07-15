#pragma once

#ifndef __SUBTITLE_ITEM_H__
#define __SUBTITLE_ITEM_H__

#include <string>
#include "ui_subtitle_item.h"
#include <QWidget>
#include <QUuid>
#include <QCheckBox>
#include "subtitle_config.h"
#include <map>
#include <QString>
#include <QVariant>
#include <curl/curl.h>
#include <sstream>
#include <nlohmann/json.hpp>
#include "Translator.h"

namespace Ui {
    class SubtitleItem;
}

/**
 * @class SubtitleItem
 * @brief Qt widget representing a single subtitle row with its timestamps and text.
 *
 * The widget bundles all user-editable fields (start/end time spin boxes, text
 * edit, selection checkbox) into one reusable component that can be placed
 * inside layouts. Each instance owns a unique identifier so that it can be
 * tracked across UI interactions such as selection or deletion.
 */
class SubtitleItem : public QWidget
{
    Q_OBJECT

private:
    Ui::SubtitleItem *ui;
    QUuid uuid;
    SubtitleConfig *subtitleConfig = new SubtitleConfig(this);

public:
    /**
     * @brief Construct a new SubtitleItem and initialize internal UI elements.
     *
     * Problem/Objectives: Create a fully configured subtitle row that can be
     * inserted into the parent layout. Each row must have a unique UUID so it
     * can be referenced later (e.g. to detect which subtitles are selected).
     *
     * Solution/Implementation: Build the designer-generated UI, enforce a
     * maximum height for compactness and generate a fresh QUuid value.
     */
    explicit SubtitleItem(QWidget *parent = nullptr);
    /**
     * @brief Destructor – Qt takes care of child widgets via parent ownership.
     */
    ~SubtitleItem();

    /** @name Order handling */
    ///@{
    /**
     * @brief Update the visual order label so that the list stays numbered.
     */
    void setOrder(int order);
    /**
     * @brief Current order index shown on UI.
     */
    int getOrder() const;
    ///@}

    /** @name Timestamp setters */
    ///@{
    void setStartHour(int hour);   /**< Set hours component of start time. */
    void setStartMinute(int minute); /**< Set minutes component of start time. */
    void setStartSecond(int second); /**< Set seconds component of start time. */
    void setStartMillisecond(int ms); /**< Set milliseconds component of start time. */
    void setEndHour(int hour);     /**< Set hours component of end time. */
    void setEndMinute(int minute); /**< Set minutes component of end time. */
    void setEndSecond(int second); /**< Set seconds component of end time. */
    void setEndMillisecond(int ms); /**< Set milliseconds component of end time. */
    ///@}

    /** @name Timestamp getters */
    ///@{
    int getStartHour() const;      /**< Get hours component of start time. */
    int getStartMinute() const;    /**< Get minutes component of start time. */
    int getStartSecond() const;    /**< Get seconds component of start time. */
    int getStartMillisecond() const; /**< Get milliseconds component of start time. */
    int getEndHour() const;        /**< Get hours component of end time. */
    int getEndMinute() const;      /**< Get minutes component of end time. */
    int getEndSecond() const;      /**< Get seconds component of end time. */
    int getEndMillisecond() const; /**< Get milliseconds component of end time. */
    ///@}

    /** @name Subtitle content */
    ///@{
    /**
     * @brief Set the text displayed in this subtitle row.
     */
    void setContent(const QString &content);
    /**
     * @brief Returns the subtitle text content.
     */
    QString getContent() const;
    ///@}

    /** @name Selection helpers */
    ///@{
    /**
     * @brief Unique identifier associated with this row.
     */
    QUuid getUuid();
    /**
     * @brief Access the selection checkbox so the parent can connect signals.
     */
    QCheckBox *getSelectedCheckbox() const;
    ///@}

    void translate();
    void textToSpeech();
};

#endif

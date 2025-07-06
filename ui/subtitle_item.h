#pragma once

#include <string>
#include "ui_subtitle_item.h"
#include <QWidget>

#ifndef __SUBTITLE_ITEM_H__
#define __SUBTITLE_ITEM_H__

namespace Ui {
    class SubtitleItem;
}

#include <QUuid>

class SubtitleItem : public QWidget
{
    Q_OBJECT

private:
    Ui::SubtitleItem *ui;
    QUuid uuid;

public:
    explicit SubtitleItem(QWidget *parent = nullptr);
    ~SubtitleItem();

    void setOrder(int order);
    void setStartHour(int hour);
    void setStartMinute(int minute);
    void setStartSecond(int second);
    void setStartMillisecond(int ms);
    void setEndHour(int hour);
    void setEndMinute(int minute);
    void setEndSecond(int second);
    void setEndMillisecond(int ms);
    void setContent(const QString &content);
    QUuid getUuid();
    QCheckBox *getSelectedCheckbox() const;
    int getOrder() const;
    int getStartHour() const;
    int getStartMinute() const;
    int getStartSecond() const;
    int getStartMillisecond() const;
    int getEndHour() const;
    int getEndMinute() const;
    int getEndSecond() const;
    int getEndMillisecond() const;
    QString getContent() const;
};

#endif

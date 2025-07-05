#pragma once

#include <string>
#include "ui_subtitle_item.h"
#include <QWidget>

#ifndef __SUBTITLE_ITEM_H__
#define __SUBTITLE_ITEM_H__

namespace Ui {
    class SubtitleItem;
}

class SubtitleItem : public QWidget
{
    Q_OBJECT

private:
    Ui::SubtitleItem *ui;

public:
    explicit SubtitleItem(QWidget *parent = nullptr);
    ~SubtitleItem();

    void setOrder(int order);
};

#endif

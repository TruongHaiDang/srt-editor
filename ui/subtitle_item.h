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
    QUuid getUuid();
    QCheckBox *getSelectedCheckbox() const;
};

#endif

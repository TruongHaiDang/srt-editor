#include "subtitle_item.h"

SubtitleItem::SubtitleItem(QWidget* parent): QWidget(parent), ui(new Ui::SubtitleItem)
{
    ui->setupUi(this);
    setMaximumHeight(132);
}

SubtitleItem::~SubtitleItem()
{
}

void SubtitleItem::setOrder(int order)
{
    ui->orderLbl->setText(QString::fromStdString(std::to_string(order)));
}

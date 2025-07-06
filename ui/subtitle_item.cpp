#include "subtitle_item.h"

SubtitleItem::SubtitleItem(QWidget* parent): QWidget(parent), ui(new Ui::SubtitleItem)
{
    ui->setupUi(this);
    setMaximumHeight(132);
    this->uuid = QUuid::createUuid();
}

SubtitleItem::~SubtitleItem()
{
}

void SubtitleItem::setOrder(int order)
{
    ui->orderLbl->setText(QString::fromStdString(std::to_string(order)));
}

QUuid SubtitleItem::getUuid()
{
    return this->uuid;
}

QCheckBox* SubtitleItem::getSelectedCheckbox() const
{
    return ui->isSelected;
}

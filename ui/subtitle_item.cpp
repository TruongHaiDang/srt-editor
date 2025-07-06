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

int SubtitleItem::getOrder() const {
    return ui->orderLbl->text().toInt();
}

int SubtitleItem::getStartHour() const {
    return ui->startHourLbl->value();
}
int SubtitleItem::getStartMinute() const {
    return ui->startMinuteLbl->value();
}
int SubtitleItem::getStartSecond() const {
    return ui->startSecondLbl->value();
}
int SubtitleItem::getStartMillisecond() const {
    return ui->startMiliSecondLbl->value();
}
int SubtitleItem::getEndHour() const {
    return ui->endHourLbl->value();
}
int SubtitleItem::getEndMinute() const {
    return ui->endMinuteLbl->value();
}
int SubtitleItem::getEndSecond() const {
    return ui->endSecondLbl->value();
}
int SubtitleItem::getEndMillisecond() const {
    return ui->endMiliSecondLbl->value();
}
QString SubtitleItem::getContent() const {
    return ui->subtitleContent->toPlainText();
}

void SubtitleItem::setStartHour(int hour) {
    ui->startHourLbl->setValue(hour);
}
void SubtitleItem::setStartMinute(int minute) {
    ui->startMinuteLbl->setValue(minute);
}
void SubtitleItem::setStartSecond(int second) {
    ui->startSecondLbl->setValue(second);
}
void SubtitleItem::setStartMillisecond(int ms) {
    ui->startMiliSecondLbl->setValue(ms);
}
void SubtitleItem::setEndHour(int hour) {
    ui->endHourLbl->setValue(hour);
}
void SubtitleItem::setEndMinute(int minute) {
    ui->endMinuteLbl->setValue(minute);
}
void SubtitleItem::setEndSecond(int second) {
    ui->endSecondLbl->setValue(second);
}
void SubtitleItem::setEndMillisecond(int ms) {
    ui->endMiliSecondLbl->setValue(ms);
}
void SubtitleItem::setContent(const QString &content) {
    ui->subtitleContent->setPlainText(content);
}

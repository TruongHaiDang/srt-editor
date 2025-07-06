#include "ExportValidator.h"

ExportValidator::ExportValidator(/* args */)
{
}

ExportValidator::~ExportValidator()
{
}

bool ExportValidator::timestampValidator(SubtitleItem* item)
{
    if (!item) return false;
    // Lấy các giá trị thời gian
    int sh = item->getStartHour();
    int sm = item->getStartMinute();
    int ss = item->getStartSecond();
    int sms = item->getStartMillisecond();
    int eh = item->getEndHour();
    int em = item->getEndMinute();
    int es = item->getEndSecond();
    int ems = item->getEndMillisecond();

    // Kiểm tra phạm vi hợp lệ
    if (sh < 0 || sh > 99) return false;
    if (eh < 0 || eh > 99) return false;
    if (sm < 0 || sm > 59) return false;
    if (em < 0 || em > 59) return false;
    if (ss < 0 || ss > 59) return false;
    if (es < 0 || es > 59) return false;
    if (sms < 0 || sms > 999) return false;
    if (ems < 0 || ems > 999) return false;

    // So sánh tổng thời gian
    int startMs = ((sh * 60 * 60 + sm * 60 + ss) * 1000) + sms;
    int endMs = ((eh * 60 * 60 + em * 60 + es) * 1000) + ems;
    if (startMs >= endMs) return false;

    return true;
}

bool ExportValidator::timestampFormatValid(const QString& timeLine)
{
    // Đúng định dạng phải có dấu phẩy giữa giây và mili giây
    // VD: 00:00:01,000 --> 00:00:04,000
    // Sai: 00:00:01.000 --> 00:00:04.000
    return !timeLine.contains('.');
}

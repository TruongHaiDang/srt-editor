#include "ExportValidator.h"

/**
 * @brief Construct a new ExportValidator object.
 *
 * Problem/Objectives: Initialize the validator that will be reused wherever
 * subtitle timestamp validation is required. At the moment there is no
 * explicit state associated with the validator, therefore the constructor
 * does not perform any additional logic.
 *
 * Solution/Implementation: Provide an explicit default constructor so that
 * the intent is clear and future initialization logic can be added without
 * breaking existing code.
 */
ExportValidator::ExportValidator(/* args */)
{
}

/**
 * @brief Destroy the ExportValidator object.
 *
 * Problem/Objectives: Release any resources that might be acquired during the
 * lifetime of the validator. Currently there are none but declaring an
 * explicit destructor up-front allows future extension without altering the
 * public interface.
 *
 * Solution/Implementation: Empty destructor body because no clean-up is
 * required at the moment.
 */
ExportValidator::~ExportValidator()
{
}

/**
 * @brief Validate start and end timestamps of a SubtitleItem.
 *
 * Problem/Objectives:
 *   1. Ensure each time component (hh, mm, ss, ms) stays within the valid
 *      range accepted by the SRT specification.
 *   2. Verify that the start time is strictly less than the end time so that
 *      subtitles do not overlap in reverse order or have zero duration.
 *
 * Solution/Implementation:
 *   • Extract individual time components from the given SubtitleItem.
 *   • Perform range checks for hours (0-99), minutes (0-59), seconds (0-59)
 *     and milliseconds (0-999).
 *   • Convert the start and end timestamps to absolute millisecond values and
 *     compare them to guarantee chronological ordering.
 *
 * @param item Pointer to the SubtitleItem whose timestamps are being
 *             validated. Passing nullptr returns false immediately.
 * @return true  If every component is within range AND start time < end time.
 * @return false Otherwise (including when item is nullptr).
 */
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

/**
 * @brief Verify that a SRT timeline string uses the correct comma separator.
 *
 * Problem/Objectives: The SRT format separates seconds and milliseconds with
 * a comma (",") – not a dot ("."). Using a dot would break compatibility
 * with most SRT parsers and media players. Detect such cases early to prevent
 * malformed exports.
 *
 * Solution/Implementation: Simply check whether a dot character is present in
 * the timeline string. If a dot is found the format is considered invalid.
 * A more sophisticated regexp is unnecessary at this stage because the comma
 * rule alone already catches the only supported alternative.
 *
 * @param timeLine Timeline string in the form "hh:mm:ss,mmm --> hh:mm:ss,mmm".
 * @return true  When the timeline does NOT contain a dot → format is valid.
 * @return false When the timeline does contain a dot → format is invalid.
 */
bool ExportValidator::timestampFormatValid(const QString& timeLine)
{
    // Đúng định dạng phải có dấu phẩy giữa giây và mili giây
    // VD: 00:00:01,000 --> 00:00:04,000
    // Sai: 00:00:01.000 --> 00:00:04.000
    return !timeLine.contains('.');
}

#ifndef __EXPORT_VALIDATOR_H__
#define __EXPORT_VALIDATOR_H__

#include "subtitle_item.h"
#include <QString>

/**
 * @class ExportValidator
 * @brief Provides helpers to verify subtitle timestamp correctness before exporting.
 */
class ExportValidator
{
private:
    /* data */
public:
    /**
     * @brief Construct a new ExportValidator object.
     */
    ExportValidator(/* args */);
    /**
     * @brief Destroy the ExportValidator object.
     */
    ~ExportValidator();

    /**
     * @brief Validate if the timestamps inside the given SubtitleItem are within valid range and chronological order.
     */
    bool timestampValidator(SubtitleItem* item);
    /**
     * @brief Check if a SRT timeline string follows the correct comma separator format.
     */
    bool timestampFormatValid(const QString& timeLine);
};

#endif
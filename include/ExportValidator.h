#ifndef __EXPORT_VALIDATOR_H__
#define __EXPORT_VALIDATOR_H__

#include "subtitle_item.h"

class ExportValidator
{
private:
    /* data */
public:
    ExportValidator(/* args */);
    ~ExportValidator();

    bool timestampValidator(SubtitleItem* item);
    bool timestampFormatValid(const QString& timeLine);
};

#endif
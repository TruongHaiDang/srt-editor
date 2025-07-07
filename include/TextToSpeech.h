#pragma once

#ifndef __TEXT_TO_SPEECH_H__
#define __TEXT_TO_SPEECH_H__

#include <QString>

class TextToSpeech
{
private:

public:
    TextToSpeech();
    ~TextToSpeech();

    void openaiConfig();
    void openaiTextToSpeech(QString text, QString outputDir);

    void elevenlabsConfig();
    void elevenlabsTextToSpeech(QString text, QString outputDir);
};

#endif

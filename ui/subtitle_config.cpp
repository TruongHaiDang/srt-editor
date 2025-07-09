#include "subtitle_config.h"

SubtitleConfig::SubtitleConfig(QWidget* parent): QDialog(parent), ui(new Ui::SubtitleConfig)
{
    ui->setupUi(this);
}

SubtitleConfig::~SubtitleConfig()
{
}

#pragma once

#include <QtWidgets/QDialog>

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QFile>

class QLabel;
class QPushButton;

class AboutWindow final : public QDialog
{
    Q_OBJECT

public:
    explicit AboutWindow(QWidget* parent = nullptr);
    ~AboutWindow() override = default;

private:
    void applyStyle();
    void buildUi();
};
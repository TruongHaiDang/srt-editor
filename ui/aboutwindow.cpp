#include "aboutwindow.h"
#include "configure.h"
#include <qdebug.h>

AboutWindow::AboutWindow(QWidget* parent)
    : QDialog(parent)
{
    buildUi();
    applyStyle();
}

void AboutWindow::buildUi()
{
    setObjectName("aboutWindow");
    setWindowTitle("About");
    setModal(true);
    setFixedSize(360, 260);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(20, 18, 20, 16);
    rootLayout->setSpacing(6);

    auto* titleLabel = new QLabel(APP_NAME, this);
    titleLabel->setObjectName("aboutTitleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    auto* versionLabel = new QLabel(QString("Version: %1").arg(APP_VERSION), this);
    versionLabel->setObjectName("aboutVersionLabel");
    versionLabel->setAlignment(Qt::AlignCenter);

    auto* buildLabel = new QLabel(
        QString("Build: %1.%2.%3")
            .arg(APP_VERSION_MAJOR)
            .arg(APP_VERSION_MINOR)
            .arg(APP_VERSION_PATCH),
        this
    );
    buildLabel->setObjectName("aboutBuildLabel");
    buildLabel->setAlignment(Qt::AlignCenter);

    auto* descriptionLabel = new QLabel("Simple SRT subtitle editor built with Qt6.", this);
    descriptionLabel->setObjectName("aboutDescriptionLabel");
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setWordWrap(true);

    auto* authorLabel = new QLabel("Author: Trương Hải Đăng", this);
    authorLabel->setObjectName("aboutAuthorLabel");
    authorLabel->setAlignment(Qt::AlignCenter);

    auto* emailLabel = new QLabel("Email: haidanghth910@gmail.com", this);
    emailLabel->setObjectName("aboutEmailLabel");
    emailLabel->setAlignment(Qt::AlignCenter);

    auto* websiteLabel = new QLabel("Website: truonghaidang.com", this);
    websiteLabel->setObjectName("aboutWebsiteLabel");
    websiteLabel->setAlignment(Qt::AlignCenter);

    auto* closeButton = new QPushButton("Close", this);
    closeButton->setObjectName("aboutCloseButton");
    closeButton->setFixedWidth(90);
    connect(closeButton, &QPushButton::clicked, this, &AboutWindow::accept);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 8, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(versionLabel);
    rootLayout->addWidget(buildLabel);

    rootLayout->addSpacing(10);
    rootLayout->addWidget(descriptionLabel);

    rootLayout->addSpacing(12);
    rootLayout->addWidget(authorLabel);
    rootLayout->addWidget(emailLabel);
    rootLayout->addWidget(websiteLabel);

    rootLayout->addStretch();
    rootLayout->addLayout(buttonLayout);
}

void AboutWindow::applyStyle()
{
    QFile file(":/css/aboutwindow.qss");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << file.errorString();
        return;
    }

    QTextStream stream(&file);
    this->setStyleSheet(stream.readAll());
}

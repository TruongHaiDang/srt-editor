#include <QtCore/QCoreApplication>
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("SubtitleEditPro");
    QCoreApplication::setApplicationName("SubtitleEdit Pro");

    MainWindow window;

    window.move(
        QGuiApplication::primaryScreen()->availableGeometry().center()
        - window.rect().center()
    );

    window.show();

    return app.exec();
}

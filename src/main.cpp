#include "main.h"
#include "mainwindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MainWindow window;
    QScreen *screen = QApplication::primaryScreen();
    QRect geometry = screen->availableGeometry();
    int x = (geometry.width() - window.width()) / 2;
    int y = (geometry.height() - window.height()) / 2;
    window.move(x, y);
    window.show();
    return app.exec();
}

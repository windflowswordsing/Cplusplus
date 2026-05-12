#include <QApplication>
#include "gamewidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GameWidget w;
    w.setWindowTitle("千年迟赴：文明灰烬");
    w.resize(1280, 720);
    w.show();
    return a.exec();
}

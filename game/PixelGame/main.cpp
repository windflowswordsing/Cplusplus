#include "GameView.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    GameView view;
    view.setWindowTitle("像素风平台跳跃 Demo - 主人专属");
    view.resize(1024, 576);
    view.show();

    return a.exec();
}

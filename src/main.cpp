#include <QApplication>
#include <QPushButton>
#include <QThread>
#include <QDebug>
#include "player.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Player player;
    player.show();
    return QApplication::exec();
}

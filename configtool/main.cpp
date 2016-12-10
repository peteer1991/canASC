#include "controll.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Controll w;
    w.show();

    return a.exec();
}

#include "rotorc.h"
#include <QApplication>
// tx dialoge
#include "txbar.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Rotorc w;
    //w.show();

    w.showFullScreen();


    return a.exec();
}


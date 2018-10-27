#include "vajegangui.h"
#include "login.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VajeganGUI w;
//    login w;
//    w.show();
    w.showMaximized();
//    w.hide();


    return a.exec();
}

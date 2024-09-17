#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    if(!w.checkUpdate()){
        w.show();
        return a.exec();
    } else {
        exit(0);
    }
}

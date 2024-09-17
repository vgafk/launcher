#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    if(!w.setBase()){
        w.show();
        return a.exec();
    } else {
        w.checkUpdate();
        w.runExecutable();
        exit(0);
    }
}

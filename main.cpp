#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    // } else {
    if(w.checkSqlBase()){
        w.checkFiles();
        if(w.runExecutable())
            exit(0);
    }
    return a.exec();
}

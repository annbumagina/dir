#include "mainwindow.h"
#include <QApplication>

#include <QMetaType>

int main(int argc, char *argv[])
{

    qRegisterMetaType< std::vector< std::vector<QString> > >("std::vector< std::vector<QString> >");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

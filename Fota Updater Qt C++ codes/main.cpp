#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPixmap pixmap(":/splash.jpg");
    QSplashScreen splash(pixmap);
    splash.show();
    for (int i = 0; i < 500000000; ++i) {};

    splash.close();
    MainWindow w;
    w.show();

    return a.exec();
}

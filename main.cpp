#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);
    a.setStyleSheet("QToolTip { color: #000000; background-color: #FFFFE0; border: 1px; }");
    MainWindow w;
    w.show();

    return a.exec();
}


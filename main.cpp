#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

/*
 * dataFileName set after finished reading or after save settings
 *
 *

void MainWindow::createDockWidgetForIndexes()
void MainWindow::add_index_pixmap(int num)
void MainWindow::itemClickedChannelList(QListWidgetItem *lwItem)
void MainWindow::itemClickedIndexList(QListWidgetItem *lwItem)

*/


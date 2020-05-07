#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

/* пока надо отладить синхронизацию изображенией,
 * проблемы могут быть в методах:

void MainWindow::createDockWidgetForIndexes()
void MainWindow::add_index_pixmap(int num)

void MainWindow::itemClickedChannelList(QListWidgetItem *lwItem)
void MainWindow::itemClickedIndexList(QListWidgetItem *lwItem)

не забыть сохранить вывод каналов шаг 2  5   10

*/


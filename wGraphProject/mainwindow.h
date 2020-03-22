#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "zscene.h"
#include "zview.h"

#include <QDockWidget>
#include <QListWidget>
#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    zView *view;
    int dnum = 0;
    void closeEvent(QCloseEvent *event);
    QDockWidget *itemslistdock = new QDockWidget(this);
    QDockWidget *dock = new QDockWidget(this);
    QListWidget *itemsList;
public slots:
    void createDock();
    void writeSettings();
    void readSettings();
    void updateDockItemsList();
    void itemClicked(QListWidgetItem *item);
    void visibilityChangedDock(bool visible);
    void setVisibleAllDocks();

};

#endif // MAINWINDOW_H

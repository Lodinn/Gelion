#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "zscene.h"
#include "zview.h"

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
public slots:
    void createDock();
    void writeSettings();
    void readSettings();

};

#endif // MAINWINDOW_H

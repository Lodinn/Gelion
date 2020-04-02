#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ImageHandler.h"
#include "qcustomplot.h"
#include "view.h"

#include <QGraphicsScene>
#include <QMainWindow>
#include <QProgressDialog>
#include <QThread>

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();
 private slots:
  void open();
 public slots:
  void show_progress(int max_progress);
  void stop_reading_file();
  void delete_progress_dialog();
  void add_envi_hdr_pixmap();
  void show_profile(QPointF point, int id = -1);
  void createDockWidgetForItem(zGraph *item);
  void winZGraphList();
  void channelList();
  void listWidgetClicked(QListWidgetItem *item);
  void listWidgetDoubleClicked(QListWidgetItem *item);
  void winZGraphProfilesShowAll();
  void winZGraphProfilesHideAll();
  void toggleViewAction(bool b);
  void createDockWidgetForChannels();
  void itemClickedChannelList(QListWidgetItem *lwItem);

 signals:
  void read_file(QString);

 private:
  void SetupUi();

 private:
  QGraphicsScene *scene;
  gQGraphicsView *view;
  QGraphicsPixmapItem *mainPixmap;
  void createActions();
  void createStatusBar();
  QThread *worker_thread = new QThread;
  ImageHandler *im_handler = new ImageHandler();
  QProgressDialog *progress_dialog = nullptr;
  QCustomPlot *wPlot;
  QVector<QCustomPlot *> plots;
protected:
  QDockWidget *dockZGraphList = new QDockWidget("Список Объектов", this);
  QListWidget *listWidget = new QListWidget(dockZGraphList);
  QDockWidget *dockChannelList = new QDockWidget("Список Каналов", this);
  QListWidget *chListWidget = new QListWidget(dockChannelList);
  void createConstDockWidgets();
};

#endif  // MAINWINDOW_H

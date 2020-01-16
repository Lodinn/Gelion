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
 signals:
  void read_file(QString);

 private:
  void SetupUi();

 private:
  QGraphicsScene *scene;
  gQGraphicsView *view;
  void createActions();
  void createStatusBar();
  QThread *worker_thread = new QThread;
  ImageHandler *im_handler = new ImageHandler();
  QProgressDialog *progress_dialog = nullptr;
  QCustomPlot *wPlot;
  QVector<QCustomPlot *> plots;
};

#endif  // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ImageHandler.h"
#include "SpectralImage.h"
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
  void setZGraphDockToggled(zGraph *item);
  void winZGraphList();
  void channelList();
  void listWidgetClicked(QListWidgetItem *item);
  void listWidgetDoubleClicked(QListWidgetItem *item);
  void winZGraphProfilesShowAll();
  void winZGraphProfilesHideAll();
  void toggleViewAction(bool b);
  void createDockWidgetForChannels();
  void itemClickedChannelList(QListWidgetItem *lwItem);
  void zGparhEditDialog(QListWidgetItem *item, zGraph *it);

 signals:
  void read_file(QString);
 protected:
  qreal qmainWindowScale = 0.6;
 private:
  void SetupUi();
  QString appName = "Gelion";
  bool saveSettingAtCloseApp = true;
  bool restoreSettingAtStartUp = true;
  QString dataFileName = "";
  void saveSettings(QString fname);
  void restoreSettings(QString fname);
  void restoreSettingsVersionOne(QSettings &settings);
  void restoreSettingsVersionTwo(QSettings &settings);
private:
    // Event handlers
  void closeEvent(QCloseEvent *event);
  QPointF getPointFromStr(QString str);
  QVector<double> getVectorFromStr(QString str);
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
protected:
  QDockWidget *dockZGraphList = new QDockWidget("Список Объектов", this);
  QListWidget *listWidget = new QListWidget(dockZGraphList);
  QDockWidget *dockChannelList = new QDockWidget("Список Каналов", this);
  QListWidget *chListWidget = new QListWidget(dockChannelList);
  void createConstDockWidgets();
};

#endif  // MAINWINDOW_H

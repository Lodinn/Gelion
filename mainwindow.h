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
  void indexList();
  void channelList();
  void listWidgetClicked(QListWidgetItem *item);
  void listWidgetDoubleClicked(QListWidgetItem *item);
  void listWidgetDeleteItem(zGraph *item);
  void winZGraphProfilesShowAll();
  void winZGraphProfilesHideAll();
  void toggleViewAction(bool b);
  void createDockWidgetForChannels();
  void createDockWidgetForIndexes();
  void itemClickedChannelList(QListWidgetItem *lwItem);
  void itemClickedIndexList(QListWidgetItem *lwItem);
  void zGparhEditDialog(QListWidgetItem *item, zGraph *it);
  void OpenRecentFile();
  void add_index_pixmap(int);
 signals:
  void read_file(QString);
  void index_calculate(QString);
 protected:
  qreal qmainWindowScale = .5;  // 0.84;
 private:
  void SetupUi();
  QString appName = "Gelion";
  bool saveSettingAtCloseApp = false;
  bool restoreSettingAtStartUp = false;
  QString dataFileName = "";
  QStringList recentFileNames;
  void saveSettings(QString fname);
  void restoreSettings(QString fname);
  void restoreSettingsVersionOne(QSettings &settings);
  void restoreSettingsVersionTwo(QSettings &settings);
  void updateNewDataSet();
  void addRecentFile();
private:
    // Event handlers
  void closeEvent(QCloseEvent *event);
  QPointF getPointFromStr(QString str);
  QVector<double> getVectorFromStr(QString str);
 private:
  QGraphicsScene *scene;
  gQGraphicsView *view;
  QMenu *fileMenu;
  QToolBar *fileToolBar;
  QAction *addSeparatorRecentFile;
  void createActions();
  void createStatusBar();
  QThread *worker_thread = new QThread;
  ImageHandler *im_handler = new ImageHandler();
  QProgressDialog *progress_dialog = nullptr;
  QCustomPlot *wPlot;
  QVector<QCustomPlot *> plots;
  struct RGB_CANNELS {
      double red = 641.0;
      double green = 550.0;
      double blue = 460.0;
  } rgb_default;
protected:
  QDockWidget *dockZGraphList = new QDockWidget("Области интереса", this);
  QListWidget *zGraphListWidget = new QListWidget(dockZGraphList);
  QDockWidget *dockIndexList = new QDockWidget("Изображения", this);
  QListWidget *indexListWidget = new QListWidget(dockIndexList);
  QDockWidget *dockChannelList = new QDockWidget("Список Каналов", this);
  QListWidget *chListWidget = new QListWidget(dockChannelList);
  QAction *showChList_All_Act = new QAction("Отображать все каналы", this);
  QAction *showChList_10_Act = new QAction("Отображать каналы с шагом 10", this);
  void createConstDockWidgets();
  QVector<QAction *> show_channel_list_acts;
  QVector<QAction *> show_zgraph_list_acts;
public slots:
  void showContextMenuChannelList(const QPoint &pos);
  void showContextMenuZGraphList(const QPoint &pos);
  void show_channel_list();
  void show_zgraph_list();
  void inputIndexDlgShow();
};

#endif  // MAINWINDOW_H

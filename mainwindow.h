#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ImageHandler.h"
#include "SpectralImage.h"
#include "qcustomplot.h"
#include "view.h"
#include "imagespectral.h"
#include "imagehistogram.h"

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
  void folder();
  void folder_debug();
 public slots:
  void show_progress(int max_progress);
  void stop_reading_file();
  void delete_progress_dialog();

  void histogramSlot();  // гистограмма
  void spectralSlot();  // спектральный анализ

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
  void restoreIndexListWidget();
  void itemClickedChannelList(QListWidgetItem *lwItem);
  void itemClickedIndexList(QListWidgetItem *lwItem);
  void zGparhEditDialog(zGraph *item);
  void OpenRecentFile();

  void add_envi_hdr_pixmap();
  void add_index_pixmap(int);

  void createDockWidgetForItem(zGraph *item);
  void setZGraphDockToggled(zGraph *item);
  void show_profile(QPointF point, int id = -1);
private:
  void updateNewDataSet(bool index_update);  // ----------------
  QStringList recentFileNames;
  void saveSettings();
  void restoreIndexes();
  void restoreSettings();
  void restoreSettingsVersionOne(QSettings &settings);
  void restoreTRUEdockWidgetsPosition();  // restoreGeometry !!! путает последовательность окон профилей
  void restoreSettingsVersionTwo(QSettings &settings);
  void addRecentFile();
  void saveGLOBALSettings();
  QString getGlobalIniFileName();

 protected:
  qreal qmainWindowScale =  .5;  // 0.84;
  void create_default_RGB_image();  // создание цветного изображения по умолчанию
 private:
  QString dataFileName = "";
  QString appName = "Gelion";
  bool saveSettingAtCloseApp = false;  // ---------
  bool restoreSettingAtStartUp = false; // ---
  QString save_slices = "_slices_.bin";
  QString save_images = "_images_.png";
  QString save_formulas = "_formulas_.bin";
  QString save_brightness = "_brightness_.bin";
  QStringList save_file_names = QStringList() << save_slices << save_images
                                              << save_formulas << save_brightness;
private:
// Event handlers
  void closeEvent(QCloseEvent *event);
  bool eventFilter(QObject *object, QEvent *event);
  void set_action_enabled(bool enable);  // action_enabled
  QPointF getPointFromStr(QString str);
  QPoint getPoint__fromStr(QString str);
  QVector<double> getVectorFromStr(QString str);
 private:
  QGraphicsScene *scene;
  gQGraphicsView *view;
  QMenu *fileMenu;
  QToolBar *fileToolBar;
  QAction *addSeparatorRecentFile;
  QAction *indexAct;
  QAction *spectralAct;
  QAction *histogramAct;

  //  QAction *maskAct;

  void createActions();
  void createStatusBar();
  void createMainConnections();
  void createConstDockWidgets();
  void SetupUi();
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

signals:
 void read_file(QString);
 void index_calculate(QString);

protected:
  QDockWidget *dockZGraphList = new QDockWidget("Области интереса", this);
  QListWidget *zGraphListWidget = new QListWidget(dockZGraphList);
  QDockWidget *dockIndexList = new QDockWidget("Изображения", this);
  QListWidget *indexListWidget = new QListWidget(dockIndexList);
  QDockWidget *dockChannelList = new QDockWidget("Список Каналов", this);
  QListWidget *chListWidget = new QListWidget(dockChannelList);
  imageSpectral *imgSpectral = new imageSpectral(this);  // окно список профилей
  imageHistogram *imgHistogram = new imageHistogram(this);  // окно отображения индексного изображения
  QVector<QAction *> show_channel_list_acts;
  QVector<QAction *> show_zgraph_list_acts;
  QSlider *slider;
  void set_abstract_index_pixmap();
  void show_channel_list_update();
  void show_histogram_widget();
  void updateHistogram();
  void calculateVisibleZObject(bool rescale);

public slots:
  void showContextMenuChannelList(const QPoint &pos);
  void showContextMenuZGraphList(const QPoint &pos);
  void show_channel_list();
  void show_zgraph_list();
  void inputIndexDlgShow();
  void settingsDlgShow();
  void change_brightness(int value);

};

#endif  // MAINWINDOW_H

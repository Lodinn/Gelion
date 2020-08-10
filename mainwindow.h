#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ImageHandler.h"
#include "SpectralImage.h"
#include "qcustomplot.h"
#include "view.h"
#include "spectralplot.h"
#include "imagehistogram.h"

#include <QGraphicsScene>
#include <QMainWindow>
#include <QProgressDialog>
#include <QThread>
#include <imagemask.h>

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();
 private:
  QString makeWritableLocation();
 private slots:
  void open();
  void folder();
  void save();                      // main save
  void folder_debug();
 public slots:
  void show_progress(int max_progress);
  void stop_reading_file();
  void delete_progress_dialog();

  void histogramSlot();  // гистограмма
  void spectralSlot();  // спектральный анализ

  void winZGraphList();
  void indexListUpdate();
  void channelListUpdate();
  void maskListUpdate();

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
  void add_mask_pixmap(J09::maskRecordType *am);

  void createDockWidgetForItem(zGraph *item);
  void setZGraphDockToggled(zGraph *item);
  void show_profile(QPointF point, int id = -1);
private:
  const QIcon icon256 = QIcon::fromTheme("mainwindow", QIcon(":/icons/256_colors.png"));
  void updateNewDataSet(bool index_update);  // ----------------
  QStringList recentFileNames;
  void saveSettings();
  QString getDataSetPath();
  void saveSettingsZ(QSettings &settings, bool save_checked_only);  // save Z Objects
  void restoreIndexes();
  void restoreSettings();
  void loadMainSettings();
  void saveMainSettings();
  QString getMainSettingsFileName();
  void restoreSettingsVersionOne(QSettings *settings);
  void createZGraphItemFromGroups(QStringList groups, QSettings *settings);  // создать объекты из текстового списка
  void restoreTRUEdockWidgetsPosition();  // restoreGeometry !!! путает последовательность окон профилей
  void restoreDockWidgetsPositionExt();  // // восстанавливает коррдинаты окон для загруженных ОИ
  void restoreSettingsVersionTwo(QSettings *settings);
  void addRecentFile();
  void saveGLOBALSettings();
  QString getGlobalIniFileName();
  void calculateIndexAndStoreToIndexList(QString title, QString formula);

 protected:
  qreal qmainWindowScale =  .84;  // 0.84;
  int desktop_width, desktop_height;
  bool saveSettingAtCloseApp = false;  // ---------
  bool restoreSettingAtStartUp = false; // ---
// global data settings
  QVector<J09::indexType> indexList;
  QVector<QAction *> predefined_index_list_acts;
  J09::globalSettingsType GLOBAL_SETTINGS;  // ГЛОБАЛЬНЫЕ НАСТРОЙКИ

  void create_default_RGB_image();  // создание цветного изображения по умолчанию
 private:
  QString dataFileName = "";
  QString appName = "Gelion";
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
  QPixmap mainPixmap;
  QMenu *fileMenu;
  QToolBar *fileToolBar;
  QAction *addSeparatorRecentFile;
  QAction *indexAct;
  QMenu *index_quick_menu;
  QAction *spectralAct;
  QAction *histogramAct;
  QAction *maskAct = new QAction(QIcon(":/icons/calculator.png"), "Калькулятор масочных изображений");

  void createActions();
  void createMainConnections();
  void createConstDockWidgets();
  void SetupUi();
  void createStatusBar();

  QThread *worker_thread = new QThread;
  ImageHandler *im_handler = new ImageHandler();
  QProgressDialog *progress_dialog = nullptr;
  QCustomPlot *wPlot;
  QVector<QCustomPlot *> plots;
  J09::RGB_CANNELS rgb_default;

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
  SpectralPlot *imgSpectral = new SpectralPlot(this);  // окно список профилей
  imageHistogram *imgHistogram = new imageHistogram(this);  // окно отображения индексного изображения
  QVector<QAction *> show_channel_list_acts;
  QVector<QAction *> show_zgraph_list_acts;
  QVector<QAction *> show_index_list_acts;
  QVector<QAction *> mask_list_acts;
  QVector<QAction *> show_mask_list_acts;
  QSlider *slider;
  QDockWidget *dockMaskImage = new QDockWidget("Маски", this);
  QListWidget *maskListWidget = new QListWidget(dockMaskImage);
  imageMask *imgMasks = new imageMask(this);  // окно КАЛЬКУЛЯТОРА МАСОК

  void set_abstract_index_pixmap();
  void show_channel_list_update();
  void show_histogram_widget();
  void updateHistogram();
  void calculateVisibleZObject(bool rescale);
 void createContextAction(const QIcon &icon, const QString &text, int num, QVector<QAction *> &av, const char *member);

private slots:
  void showContextMenuMaskImagList(const QPoint &pos);
  void showContextMenuChannelList(const QPoint &pos);
  void showContextMenuZGraphList(const QPoint &pos);
  void showContextMenuDockIndexList(const QPoint &pos);
  void show_mask_list();
  void show_index_list();
  void show_channel_list();
  void show_zgraph_list();
  void inputIndexDlgShow();
  void predefined_index_menu();
  void settingsDlgShow();
  void change_brightness(int value);
  void spectralUpdateExt(zGraph *item);

};

#endif  // MAINWINDOW_H

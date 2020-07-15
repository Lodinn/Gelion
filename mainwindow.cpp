#include "mainwindow.h"
#include "zgraphparamdlg.h"
#include "ui_zgraphparamdlg.h"
#include "inputindexdlg.h"
#include "ui_inputindexdlg.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QMenu>
#include <QToolBar>
#include <QtWidgets>
#include <QDebug>

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
//    if (object == zGraphListWidget) qDebug() << "event" << event;
    return QMainWindow::eventFilter(object, event);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

  setWindowTitle(QString("%1").arg(appName));
  QApplication::setApplicationName(appName);

  loadMainSettings();

  SetupUi();

  im_handler->moveToThread(worker_thread);

//  installEventFilter(this);

  imgSpectral->rainbowCheckBox->setChecked(GLOBAL_SETTINGS.zobjects_prof_rainbow_show);

//    QCoreApplication.setApplicationName(ORGANIZATION_NAME)
//    QCoreApplication.setOrganizationDomain(ORGANIZATION_DOMAIN)

}

MainWindow::~MainWindow() {
  delete scene;
  delete view;
  worker_thread->wait(200);
  delete im_handler;
  delete worker_thread;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event)
  if (saveSettingAtCloseApp) {
      saveSettings();
      im_handler->save_settings_all_images(save_file_names);
  }  // if
  GLOBAL_SETTINGS.zobjects_prof_rainbow_show = imgSpectral->rainbowCheckBox->isChecked();
  saveMainSettings();
}

void MainWindow::set_action_enabled(bool enable)
{
//    view->localFolderAct->setEnabled(enable);
    view->pointAct->setEnabled(enable);
    view->rectAct->setEnabled(enable);
    view->ellipseAct->setEnabled(enable);
    view->polylineAct->setEnabled(enable);
    view->polygonAct->setEnabled(enable);
    indexAct->setEnabled(enable);
    index_quick_menu->setEnabled(enable);
//    spectralAct->setEnabled(enable);
//    histogramAct->setEnabled(enable);
}

QString MainWindow::getDataSetPath()
{
    if (dataFileName.isEmpty()) return QString();
    QFileInfo info(dataFileName);
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    writableLocation += "/" + info.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    return writableLocation;
}

void MainWindow::saveSettings()
{
    if (dataFileName.isEmpty()) return;
    QFileInfo info(dataFileName);
    QString iniFileName = info.completeBaseName() + ".ini";
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    writableLocation += "/" + info.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    iniFileName = writableLocation + "/" + iniFileName;
    QSettings settings( iniFileName, QSettings::IniFormat );
    QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
    settings.setIniCodec( codec );
    settings.clear();
    settings.setValue("version", 1);
    settings.setValue("scale", view->GlobalScale);
    settings.setValue("rotation", view->GlobalRotate);
    settings.setValue("channelNum", view->GlobalChannelNum);
    settings.setValue("channelStep", view->GlobalChannelStep);
    settings.setValue("horizontal", view->horizontalScrollBar()->value());
    settings.setValue("vertical", view->verticalScrollBar()->value());
    settings.setValue("dockZGraphList", dockZGraphList->isVisible());
    settings.setValue("dockChannelList", dockChannelList->isVisible());
    settings.setValue("dockIndexList", dockIndexList->isVisible());
    settings.setValue("indexCount", indexListWidget->count());  // количество расчетных изображений

    saveSettingsZ(settings, false);

    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState(1));

}

void MainWindow::saveSettingsZ(QSettings &settings, bool save_checked_only)
{
    auto graphList = view->getZGraphItemsList();
    int num = 0;
    foreach(zGraph *zg, graphList) {
        if (save_checked_only && !zg->isVisible()) continue;
        QStringList strlist = zg->getSettings(num);
        foreach(QString str, strlist) {
            int index = str.indexOf(zg->setsep);
            if (index == -1) continue;
            QString mid_Key = str.mid(0,index);
            QString mid_Value = str.mid(index+1);
            settings.setValue(mid_Key, mid_Value);
        }  // for
        num++;
    }  // for
}

void MainWindow::SetupUi() {
  setWindowTitle("Gelion");
  QRect desktop = QApplication::desktop()->screenGeometry();
  int sw = desktop.width();  int sh = desktop.height();
  desktop_width = sw;  desktop_height = sh;
  int left = round(sw*(1-qmainWindowScale)*0.5+200);
  int top = round(sh*(1-qmainWindowScale)*0.5);
  int width = round(sw*qmainWindowScale*.75);
  int height = round(sh*qmainWindowScale);
  move( left, top );
  resize( width, height );

  setWindowIcon(icon256);

  scene = new QGraphicsScene(this);
  view = new gQGraphicsView(scene);
  view->imgHand = im_handler;
  setCentralWidget(view);
  QPixmap test(QString(":/images/view.jpg"));
  view->mainPixmap = scene->addPixmap(test);  // p->type(); enum { Type = 7 };
  view->mainPixmap->setOpacity(0.7);

  createActions();
  createStatusBar();
  createConstDockWidgets();
  createMainConnections();

}

void MainWindow::createActions() {

// &&&file
  fileMenu = menuBar()->addMenu(tr("&Файл"));
  fileMenu->addAction(view->openAct);
  connect(view->openAct, &QAction::triggered, this, &MainWindow::open);
  fileMenu->addAction(view->localFolderAct);
  connect(view->localFolderAct, &QAction::triggered, this, &MainWindow::folder);
  view->saveAct->setShortcut(QKeySequence::SaveAs);                             // main save
  fileMenu->addAction(view->saveAct);
  connect(view->saveAct, &QAction::triggered, this, &MainWindow::save);
  fileMenu->addAction(view->debugFolderAct);
  connect(view->debugFolderAct, &QAction::triggered, this, &MainWindow::folder_debug);
  fileMenu->addSeparator();
  addSeparatorRecentFile = fileMenu->addSeparator();  // для дальнейшей вставки списка файлов
  fileMenu->addAction(view->closeAct);
  connect(view->closeAct, &QAction::triggered, this, &MainWindow::close);
  fileToolBar = addToolBar(tr("Файл"));  fileToolBar->setObjectName("fileToolBar");
  fileToolBar->addAction(view->openAct);
  fileToolBar->addAction(view->localFolderAct);
  fileToolBar->addAction(view->saveAct);                        // main save
  fileToolBar->addSeparator();
// &&& inset ROI menu
  QMenu *itemsMenu = menuBar()->addMenu("&Области интереса");
  QToolBar *itemsToolBar = addToolBar("Области интереса");
  itemsToolBar->setObjectName("itemsToolBar");
  itemsMenu->addAction(view->pointAct);   itemsToolBar->addAction(view->pointAct);
  itemsMenu->addAction(view->rectAct);    itemsToolBar->addAction(view->rectAct);
  itemsMenu->addAction(view->ellipseAct); itemsToolBar->addAction(view->ellipseAct);
  itemsMenu->addAction(view->polylineAct);    itemsToolBar->addAction(view->polylineAct);
  itemsMenu->addAction(view->polygonAct); itemsToolBar->addAction(view->polygonAct);
// &&&index
    QMenu *indexMenu = menuBar()->addMenu(tr("&Индексы"));
    QToolBar *indexToolBar = addToolBar(tr("Индексы"));
    indexToolBar->setObjectName("indexToolBar");
    const QIcon indexIcon =
        QIcon::fromTheme("Добавить индексное изображение", QIcon(":/icons/MapleLeaf.png"));
    indexAct = new QAction(indexIcon, tr("&Добавить индексное изображение"), this);
    indexMenu->addAction(indexAct);   indexToolBar->addAction(indexAct);
    connect(indexAct, SIGNAL(triggered()), this, SLOT(inputIndexDlgShow()));

//    indexActExt = new QAction(indexIcon, tr("&Быстрый расчет индекса"), this);
//    indexMenu->addAction(indexActExt);
//    indexToolBar->addAction(indexActExt);
    index_quick_menu = new QMenu(tr("&Быстрый выбор индекса"), this);
    index_quick_menu->setIcon(indexIcon);
    qDebug() << "indexList count = " << indexList.count();
    foreach(J09::indexType item, indexList) {
        QAction *action = index_quick_menu->addAction(item.name);
        action->setData(item.formula);  predefined_index_list_acts.append(action);
        connect(action, SIGNAL(triggered()), this, SLOT(predefined_index_menu()));
    }  // foreach
    indexMenu->addMenu(index_quick_menu);       indexToolBar->addAction(index_quick_menu->menuAction());
    index_quick_menu->setEnabled(false);

    spectralAct = new QAction(QIcon(":/icons/full-spectrum.png"), tr("&Спектральный анализ"), this);
    spectralAct->setCheckable(true); spectralAct->setChecked(false);
    spectralAct->setEnabled(false);
    indexMenu->addAction(spectralAct);   indexToolBar->addAction(spectralAct);

    histogramAct = new QAction(QIcon(":/icons/histogram.png"), tr("&Отображать гистограмму"), this);
    histogramAct->setCheckable(true); histogramAct->setChecked(true);
    histogramAct->setEnabled(false);
    indexMenu->addAction(histogramAct);   indexToolBar->addAction(histogramAct);

// &&& windows
  QMenu *winMenu = menuBar()->addMenu("&Окна");
  QToolBar *winToolBar = addToolBar(winMenu->title());
  winToolBar->setObjectName("winToolBar");
  winMenu->addAction(view->winZGraphListAct);   winToolBar->addAction(view->winZGraphListAct);
  winMenu->addAction(view->indexListAct);   winToolBar->addAction(view->indexListAct);
  winMenu->addAction(view->channelListAct);   winToolBar->addAction(view->channelListAct);
  winMenu->addAction(view->maskListAct);   winToolBar->addAction(view->maskListAct);
  winMenu->addAction(view->winZGraphListShowAllAct);   winToolBar->addAction(view->winZGraphListShowAllAct);
  winMenu->addAction(view->winZGraphListHideAllAct);   winToolBar->addAction(view->winZGraphListHideAllAct);
  winMenu->addSeparator();      winToolBar->addSeparator();
// &&& settings
  QMenu *settingsMenu = menuBar()->addMenu("&Настройки");
  QToolBar *settingsToolBar = addToolBar(settingsMenu->title());
  settingsToolBar->setObjectName("settingsToolBar");
  slider = new QSlider(Qt::Horizontal);
  slider->setToolTip("Яркость");
  slider->setMaximum(100);  slider->setFixedSize(150,12);
  settingsToolBar->addWidget(slider);  slider->setEnabled(false);
  connect(slider, SIGNAL(valueChanged(int)), this, SLOT(change_brightness(int)));
  settingsToolBar->addSeparator();

  QAction *mainSettingsAct = new QAction(QIcon(":/icons/settings-150.png"), "&Настройки программы", this);
  settingsMenu->addAction(mainSettingsAct);
  settingsToolBar->addAction(mainSettingsAct);
  connect(mainSettingsAct, SIGNAL(triggered()), this, SLOT(settingsDlgShow()));

  set_action_enabled(false);

}

void MainWindow::createStatusBar() {
    statusBar()->showMessage("Ctrl+Скролл-масштабирование*Скролл-перемотка верт.*Alt+Скролл-перемотка гориз.*Левая кн.мышь-перемещение");
}

void MainWindow::createMainConnections()
{
// file menu

// im_handler

    connect(this, SIGNAL(read_file(QString)), im_handler,
            SLOT(read_envi_hdr(QString)), Qt::DirectConnection);
    connect(im_handler, SIGNAL(numbands_obtained(int)), this,
            SLOT(show_progress(int)), Qt::DirectConnection);
    connect(im_handler, SIGNAL(finished()), this, SLOT(add_envi_hdr_pixmap()));
    connect(im_handler, SIGNAL(finished()), this, SLOT(delete_progress_dialog()),
            Qt::DirectConnection);

    connect(this, SIGNAL(index_calculate(QString)), im_handler,
            SLOT(append_index_raster(QString)), Qt::DirectConnection);
    connect(im_handler, SIGNAL(index_finished(int)), this, SLOT(add_index_pixmap(int)));
    connect(im_handler, SIGNAL(index_finished(int)), this, SLOT(delete_progress_dialog()),
            Qt::DirectConnection);

    connect(view, SIGNAL(point_picked(QPointF)), this,
            SLOT(show_profile(QPointF)));

    connect(view,SIGNAL(insertZGraphItem(zGraph*)),this,SLOT(createDockWidgetForItem(zGraph*)));
    connect(view,SIGNAL(setZGraphDockToggled(zGraph*)),this,SLOT(setZGraphDockToggled(zGraph*)));

    connect(scene,SIGNAL(selectionChanged()),view,SLOT(selectionZChanged()));

    connect(histogramAct, SIGNAL(triggered()), this, SLOT(histogramSlot()));  // гистограмма
    connect(spectralAct, SIGNAL(triggered()), this, SLOT(spectralSlot()));  // СПЕКТРАЛЬНЫЙ АНАЛИЗ
    connect(view, SIGNAL(changeZObject(zGraph*)), this, SLOT(spectralUpdateExt(zGraph*)));

// view
    connect(view->winZGraphListAct, SIGNAL(triggered()), this, SLOT(winZGraphList()));  // области интереса
    connect(view->indexListAct, SIGNAL(triggered()), this, SLOT(indexListUpdate()));  // изображения
    connect(view->channelListAct, SIGNAL(triggered()), this, SLOT(channelListUpdate()));  // каналы
    connect(view->maskListAct, SIGNAL(triggered()), this, SLOT(maskListUpdate()));  // маски
    connect(view->winZGraphListShowAllAct, SIGNAL(triggered()), this, SLOT(winZGraphProfilesShowAll()));
    connect(view->winZGraphListHideAllAct, SIGNAL(triggered()), this, SLOT(winZGraphProfilesHideAll()));

}

void MainWindow::spectralUpdateExt(zGraph *item)
{
    if (!imgSpectral->isVisible()) return;
    auto zz_list = view->getZGraphItemsList();

    if (item == nullptr) {
        imgSpectral->updateData(dataFileName, zz_list, true);  // обновление всех профилей
        imgSpectral->update();
        return;
    }  // if

    QList<zGraph *> z_list;  foreach(zGraph *z, zz_list) if (z->isVisible()) z_list.append(z);
    int num = -1;
    foreach(zGraph *z, z_list)
        if (z == item) { num = z_list.indexOf(z); break; }
    if (num == -1) return;
    imgSpectral->updateDataOneProfile(item, num);

}

void MainWindow::createConstDockWidgets()
{
    QRect rec = QApplication::desktop()->screenGeometry();
    int scrHeignt4 = qRound(rec.height() / 4.5);
// Области интереса
    dockZGraphList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(dockZGraphList, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenuZGraphList(QPoint)));
    dockZGraphList->setWindowIcon(QIcon(":/icons/windows2.png"));
    dockZGraphList->setFloating(true);  dockZGraphList->setObjectName("dockZGraphList");
    dockZGraphList->setFixedSize(220,scrHeignt4-36);
    dockZGraphList->setWidget(zGraphListWidget);
    dockZGraphList->move(rec.width() - 250,10+0*scrHeignt4);
    addDockWidget(Qt::BottomDockWidgetArea, dockZGraphList);
    connect(dockZGraphList->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
    connect(zGraphListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listWidgetClicked(QListWidgetItem*)));
    connect(zGraphListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(listWidgetDoubleClicked(QListWidgetItem*)));
    connect(view, SIGNAL(removeFromzGraphListWidget(zGraph *)), this, SLOT(listWidgetDeleteItem(zGraph *)));
// контекстное меню для отображения областей интереса и профилей
    QAction *action = new QAction(QIcon(":/icons/profs_show.png"), "Отобразить все области интереса", this);
    action->setData(1);  show_zgraph_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));
    action = new QAction(QIcon(":/icons/profs_hide.png"), "Скрыть все области интереса", this);
    action->setData(2);  show_zgraph_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));
    action = view->winZGraphListDeleteAllAct;  // Удалить все области интереса ...
    action->setData(11);  show_zgraph_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));
    action = new QAction(this);  action->setSeparator(true);  show_zgraph_list_acts.append(action);
    action = new QAction(QIcon(":/icons/profs_show.png"), "Отобразить все профили", this);
    action->setData(3);  show_zgraph_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));
    action = new QAction(QIcon(":/icons/profs_hide.png"), "Скрыть все профили", this);
    action->setData(4);  show_zgraph_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));
    action = new QAction(this);  action->setSeparator(true);  show_zgraph_list_acts.append(action);
    action = new QAction(QIcon(":/icons/csv2.png"), "Сохранить все профили в Excel *.csv файл ...", this);
    action->setData(5);  show_zgraph_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));
    action = new QAction(QIcon(":/icons/open.png"), "Загрузить области интереса из *.roi файла ...", this);
    action->setData(6);  show_zgraph_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));
    action = new QAction(QIcon(":/icons/save.png"), "Сохранить выделенные области интереса в *.roi файл ...", this);
    action->setData(7);  show_zgraph_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));
// работа с изображениями - масками
    action = new QAction(this);  action->setSeparator(true);  show_zgraph_list_acts.append(action);
    action = new QAction(QIcon(":/icons/theater-roi.png"), "Создать маску из областей интереса ...", this);
    action->setData(8);  show_zgraph_list_acts.append(action);  // mask - 6
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));

    action = new QAction(QIcon(":/icons/theater-roi.png"), "Sun", this);
    action->setData(9);  mask_list_acts.append(action);  // mask - 7
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));
    action = new QAction(QIcon(":/icons/theater-roi.png"), "Shadow", this);
    action->setData(10);  mask_list_acts.append(action);  // mask - 8
    connect(action, SIGNAL(triggered()), this, SLOT(show_zgraph_list()));

// Изображения
    dockIndexList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(dockIndexList, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenuDockIndexList(QPoint)));
    dockIndexList->setWindowIcon(QIcon(":/icons/palette.png"));
    dockIndexList->setFloating(true);  dockIndexList->setObjectName("dockIndexList");
    dockIndexList->setFixedSize(220,scrHeignt4-36);
    dockIndexList->setWidget(indexListWidget);
    dockIndexList->move(rec.width() - 250,10+1*scrHeignt4);
    addDockWidget(Qt::BottomDockWidgetArea, dockIndexList);
    connect(indexListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClickedIndexList(QListWidgetItem*)));
    connect(dockIndexList->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
// контекстное меню для индексных изображений
    action = new QAction(QIcon(":/icons/delete-32.png"), "Удалить индексное изображение", this);
    action->setData(0);  show_index_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_index_list()));
// Список Каналов
    chListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(chListWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenuChannelList(QPoint)));
    dockChannelList->setWindowIcon(QIcon(":/icons/list-channel.jpg"));
    dockChannelList->setFloating(true);  dockChannelList->setObjectName("dockChannelList");
    dockChannelList->setFixedSize(220,scrHeignt4-36);
    dockChannelList->setWidget(chListWidget);
    dockChannelList->move(rec.width() - 250,10+2*scrHeignt4);
    addDockWidget(Qt::BottomDockWidgetArea, dockChannelList);
    connect(chListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClickedChannelList(QListWidgetItem*)));
    connect(dockChannelList->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
// контекстное меню для отображения части каналов с шагом 2-10
    action = new QAction(QIcon(":/icons/all_channels.png"), "Отображать все каналы", this);
    action->setData(1);  show_channel_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_channel_list()));
    action = new QAction(QIcon(":/icons/step2_channels.png"), "Отображать каналы с шагом 2", this);
    action->setData(2);  show_channel_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_channel_list()));
    action = new QAction(QIcon(":/icons/step5_channels.png"), "Отображать каналы с шагом 5", this);
    action->setData(5);  show_channel_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_channel_list()));
    action = new QAction(QIcon(":/icons/step10_channels.png"), "Отображать каналы с шагом 10", this);
    action->setData(10);  show_channel_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_channel_list()));

// СПЕКТРАЛЬНЫЙ АНАЛИЗ
    imgSpectral->hide();    imgSpectral->setObjectName("imgSpectral");
    addDockWidget(Qt::BottomDockWidgetArea, imgSpectral);
    connect(imgSpectral->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));

// АНАЛИЗ ИНДЕКСНЫХ ИЗОБРАЖЕНИЙ
    imgHistogram->hide();   imgHistogram->setObjectName("imgHistogram");
    addDockWidget(Qt::BottomDockWidgetArea, imgHistogram);
    connect(imgHistogram->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
//    connect(imgHistogram,SIGNAL(appendMask()),this,SLOT(appendMaskImage()));
    bool success = connect(imgHistogram,&imageHistogram::appendMask,this,&MainWindow::open);
    qDebug() << "bool success = " << success;
//    this, &MainWindow::open

// МАСКИ
    dockMaskImage->setFloating(true);  dockMaskImage->setObjectName("dockMaskImage");
    dockMaskImage->setFixedSize(220,scrHeignt4-36);
    dockMaskImage->setWidget(maskListWidget);
    dockMaskImage->move(rec.width() - 250,10+3*scrHeignt4);
    addDockWidget(Qt::BottomDockWidgetArea, dockMaskImage);
    connect(dockMaskImage->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
}

void MainWindow::appendMaskImage()
{
    qDebug() << "MainWindow::appendMaskImage()";
}

void MainWindow::show_profile(QPointF point, int id) {
  int px, py;
  px = int(point.rx());
  py = int(point.ry());
  QVector<QPointF> v = im_handler->current_image()->get_profile(QPoint(px, py));
  if (v.length() == 0) return;

  int d = im_handler->current_image()->get_bands_count();

  QVector<double> x(d), y(d);
  for (int i = 0; i < d; ++i) {
    x[i] = v[i].rx();
    y[i] = v[i].ry();
  }

  if (id < 0) {
    QDockWidget *dock = new QDockWidget(tr("Plot profile"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea |
                          Qt::BottomDockWidgetArea);
    dock->setAllowedAreas(Qt::NoDockWidgetArea);
    dock->setFixedHeight(200);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    wPlot = new QCustomPlot();
    plots.append(wPlot);
    dock->setWidget(wPlot);
    wPlot->resize(500, 200);
    wPlot->addGraph();
    // give the axes some labels:
    wPlot->xAxis->setLabel("длина волны, нм");
    wPlot->yAxis->setLabel("коэффициент отражения");
    // set axes ranges, so we see all data:
    wPlot->xAxis->setRange(390, 1010);
    wPlot->yAxis->setRange(0, 1.5);
  } else if (id < plots.count()) {
    wPlot = plots[id];
  } else {
    qDebug() << "CRITICAL - bad plot id";
    return;
  }
  wPlot->graph(0)->setData(x, y);
  wPlot->replot();
}

void MainWindow::createDockWidgetForItem(zGraph *item)
{
    spectralAct->setEnabled(true);
    QDockWidget *dockw = new QDockWidget(item->getTitle(), this);

    item->dockw = dockw;
    item->imgHand = im_handler;
    dockw->setFloating(true);  dockw->setObjectName("zGraph");
    dockw->resize(GLOBAL_SETTINGS.zobject_dock_size_w,GLOBAL_SETTINGS.zobject_dock_size_h);

    wPlot = new QCustomPlot();  dockw->setWidget(wPlot);

    wPlot->setInteraction(QCP::iRangeZoom,true);   // Включаем взаимодействие удаления/приближения
    wPlot->setInteraction(QCP::iRangeDrag, true);  // Включаем взаимодействие перетаскивания графика
    wPlot->axisRect()->setRangeDrag(Qt::Vertical);   // перетаскивание только по горизонтальной оси
    wPlot->axisRect()->setRangeZoom(Qt::Vertical);   // удаления/приближения только по горизонтальной оси
    wPlot->resize(GLOBAL_SETTINGS.zobject_dock_size_w,GLOBAL_SETTINGS.zobject_dock_size_h);

    wPlot->addGraph();
    QCPGraph *graph_lower = wPlot->addGraph();
    QCPGraph *graph_upper = wPlot->addGraph();

    graph_lower->setPen(Qt::NoPen);
    graph_lower->setSelectable(QCP::stNone);

    graph_upper->setPen(Qt::NoPen);
    item->transparency = GLOBAL_SETTINGS.std_dev_brush_color_transparency;
    QColor bc(GLOBAL_SETTINGS.std_dev_brush_color_red,GLOBAL_SETTINGS.std_dev_brush_color_green,
              GLOBAL_SETTINGS.std_dev_brush_color_blue, item->transparency);
    graph_upper->setBrush(bc);
    graph_upper->setSelectable(QCP::stNone);
    graph_upper->setChannelFillGraph(graph_lower);

// give the axes some labels:
    wPlot->xAxis->setLabel("длина волны, нм");
    wPlot->yAxis->setLabel("коэффициент отражения");
// set axes ranges, so we see all data:
    wPlot->xAxis->setRange(GLOBAL_SETTINGS.zobject_plot_xAxis_lower,GLOBAL_SETTINGS.zobject_plot_xAxis_upper);
    wPlot->yAxis->setRange(GLOBAL_SETTINGS.zobject_plot_yAxis_lower,GLOBAL_SETTINGS.zobject_plot_yAxis_upper);
    item->plot = wPlot;
    item->setContextMenuConnection();
    addDockWidget(Qt::BottomDockWidgetArea, dockw);
    dockw->hide();  dockw->setAllowedAreas(nullptr);  // NoToolBarArea - !!! не прикрепляется к главному окну
    QListWidgetItem *lwItem = new QListWidgetItem();
    item->listwidget = lwItem;
    lwItem->setText(item->getTitle());
    QFont font = lwItem->font();  font.setBold(true);
    lwItem->setFont(font);
    lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
    lwItem->setCheckState(Qt::Checked);
    lwItem->setIcon(item->aicon);
    zGraphListWidget->insertItem(0, lwItem);

    connect(item, SIGNAL(changeParamsFromDialog(zGraph*)), this, SLOT(spectralUpdateExt(zGraph*)));
    connect(item, SIGNAL(changeZObjectNative(zGraph*)), this, SLOT(spectralUpdateExt(zGraph*)));
}

void MainWindow::setZGraphDockToggled(zGraph *item)
{
    connect(item->dockw->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));

    if (imgSpectral->isVisible()) calculateVisibleZObject(true);

}

void MainWindow::winZGraphList()
{
    dockZGraphList->setVisible(!dockZGraphList->isVisible());
}

void MainWindow::indexListUpdate()
{
    dockIndexList->setVisible(!dockIndexList->isVisible());
}

void MainWindow::channelListUpdate()
{
    dockChannelList->setVisible(!dockChannelList->isVisible());
}

void MainWindow::maskListUpdate()
{
    dockMaskImage->setVisible(!dockMaskImage->isVisible());
}

void MainWindow::winZGraphProfilesShowAll()
{
    QList<zGraph *> zitemlist = view->getZGraphItemsList();
    foreach (zGraph *zitem, zitemlist)
        zitem->dockw->setVisible(true);
}

void MainWindow::winZGraphProfilesHideAll()
{
    QList<zGraph *> zitemlist = view->getZGraphItemsList();
    foreach (zGraph *zitem, zitemlist)
        zitem->dockw->setVisible(false);
}

void MainWindow::listWidgetClicked(QListWidgetItem *item)
{
    QList<zGraph *> zitems = view->getZGraphItemsList();
    zGraph *zgrap_item = nullptr;
    foreach(zGraph *it, zitems)
        if (it->listwidget == item) {
            zgrap_item = it;
        break;
        }  // if
    if (zgrap_item == nullptr) return;
    if (zgrap_item->isSelected()) {
        view->deleteGrabberRects();
        view->deleteTmpLines();
    }
    if (item->checkState() == Qt::Checked) {
        zgrap_item->setVisible(true); zgrap_item->dockw->setVisible(true);
        spectralAct->setEnabled(true);
        imgSpectral->updateData(dataFileName, zitems, true);
    }
    else { zgrap_item->setVisible(false); zgrap_item->setSelected(false);
        zgrap_item->dockw->setVisible(false);

        calculateVisibleZObject(false);  // проверим есть ли видимые объекты
    }
}

void MainWindow::zGparhEditDialog(zGraph *item)
{
    zgraphParamDlg *dlg = new zgraphParamDlg(this);
    dlg->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
    dlg->ui->buttonBox->buttons().at(1)->setText("Отмена");
    QPoint pos = dockZGraphList->geometry().bottomLeft()-QPoint(dlg->width(),0)+QPoint(dockZGraphList->width(),0);
    dlg->move(pos);
    item->setParamsToDialog(dlg);
    if (dlg->exec() == QDialog::Accepted) {
        if (dlg->ui->checkBoxZGraphDelete->isChecked()) {
            view->deleteZGraphItem(item);  return; }
        item->getParamsFromDialog(dlg);
    }  // if
}

void MainWindow::listWidgetDoubleClicked(QListWidgetItem *item)
{
    zGraph *it = nullptr;
    auto zGraphList = view->getZGraphItemsList();
    foreach(zGraph *z, zGraphList)
        if (z->listwidget == item) {
            it = z;  break;
        }  // foreach
    if (it == nullptr) return;
    zGparhEditDialog(it);
}

void MainWindow::toggleViewAction(bool b)
{
    Q_UNUSED(b)
//    QList<QGraphicsItem *> items = view->scene()->items();
    QList<zGraph *> zitemlist = view->getZGraphItemsList();
    foreach (zGraph *zitem, zitemlist) {
        auto lwItem = zGraphListWidget->item(zitemlist.indexOf(zitem));
        if (zitem->isVisible()) lwItem->setCheckState(Qt::Checked);
        else lwItem->setCheckState(Qt::Unchecked);
        QFont font = lwItem->font();
        font.setBold(zitem->dockw->isVisible());
        lwItem->setFont(font);
    }  // for
    view->winZGraphListAct->setChecked(dockZGraphList->isVisible());
    view->indexListAct->setChecked(dockIndexList->isVisible());
    view->channelListAct->setChecked(dockChannelList->isVisible());
    view->maskListAct->setChecked(dockMaskImage->isVisible());

    histogramAct->setChecked(imgHistogram->isVisible());
    spectralAct->setChecked(imgSpectral->isVisible());
}

// connect(view->openAct, &QAction::triggered, this, &MainWindow::open);
void MainWindow::open() {
  QString fname = QFileDialog::getOpenFileName(
      this, tr("Открытие файла данных"), "../data",
      tr("ENVI HDR Files (*.hdr);;ENVI HDR Files (*.hdr)"));
  if (fname.isEmpty()) return;
  QStringList fnames = im_handler->get_image_file_names();
  int num = fnames.indexOf(fname);
  if (num != -1) {
    if (im_handler->set_current_image(num)) {
      updateNewDataSet(false);
      addRecentFile();
      return;
    }  // if
  }  // if
  view->openAct->setEnabled(false);
  QApplication::processEvents();
  emit read_file(fname);
}

void MainWindow::add_envi_hdr_pixmap() {
  view->mainPixmap->setOpacity(1.0);
  view->resetMatrix();  view->resetTransform();
// dataFileName ПРИСВАИВАТЬ ПОСЛЕ ЗАГРУЗКИ ФАЙЛА !!!!!
  if (!dataFileName.isEmpty()) saveSettings();  // ****************************
  dataFileName = im_handler->current_image()->get_file_name();
  view->empty = false;  // сцена содержит данные
  view->openAct->setEnabled(true);
  updateNewDataSet(true);
  addRecentFile();
  view->openAct->setEnabled(true);
  set_action_enabled(true);
  QApplication::processEvents();
}

void MainWindow::updateNewDataSet(bool index_update)
{
    view->clearForAllObjects();
    zGraphListWidget->clear();
    indexListWidget->clear();
    chListWidget->clear();
    setWindowTitle(QString("%1 - [%2]").arg(appName).arg(dataFileName));
    createDockWidgetForChannels();
    if (restoreSettingAtStartUp && index_update) restoreIndexes();  // slice++image++brightness
    createDockWidgetForIndexes();  // with RGB default + slice++image++brightness
    if (restoreSettingAtStartUp) restoreSettings();  // dock + zgraph
//    view->localFolderAct->setEnabled(true);
    slider->setEnabled(true);
    view->openAct->setEnabled(true);
}

void MainWindow::calculateVisibleZObject(bool rescale) {

    auto z_list = view->getZGraphItemsList();  // z objects list
    int count = 0;
    foreach(zGraph *zg, z_list)
        if (zg->isVisible()) count++;
    if (count == 0) {
        imgSpectral->hide();
        spectralAct->setEnabled(false);
        spectralAct->setChecked(false);
        return;
    }  // if
    imgSpectral->updateData(dataFileName, z_list, rescale);
}

void MainWindow::listWidgetDeleteItem(zGraph *item)
{
    zGraphListWidget->update();
    calculateVisibleZObject(false);
    Q_UNUSED(item);
}

void MainWindow::folder()
{
    auto writableLocation = makeWritableLocation();
    QProcess::startDetached(QString("explorer /root,\"%1\"")
                            .arg(QDir::toNativeSeparators(writableLocation)));
}

QString MainWindow::makeWritableLocation()
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo fi(dataFileName);
    writableLocation += "/" + fi.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    return writableLocation;
}

void MainWindow::save()
{
    if (dataFileName.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowIcon(icon256);
        msgBox.setWindowTitle("Ошибка");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Для сохранения необходимо загрузить данные !");
        msgBox.exec();
        return;
    }  // if

    auto writableLocation = makeWritableLocation();

    QString fileName= QFileDialog::getSaveFileName(this, "Сохранение главного изображения", writableLocation, "Файлы PNG (*.png);;Файлы JPEG (*.jpeg);;Файлы BMP (*.bmp)" );
    if (!fileName.isNull())
    {
        QPixmap pixmap = view->grab();
        pixmap.save(fileName);
    }  // if
}

void MainWindow::folder_debug()
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    qDebug() << writableLocation;
    QDir dir(writableLocation);
    dir.removeRecursively();
}

void MainWindow::show_progress(int max_progress) {
  //    QProgressDialog *progress_dialog = new QProgressDialog("Загрузка файла
  //    ...", "Отмена", 0, max_progress, this);
  if (progress_dialog != nullptr) delete progress_dialog;
  QString pdInfoStr = "", pdTitleStr = "";
  if (im_handler->show_progress_mode == ImageHandler::hdr_mode) {
      pdTitleStr = "Открыть файл";  pdInfoStr = "Загрузка файла ...";
      view->openAct->setEnabled(true);
      QApplication::processEvents();
  }  // if
  if (im_handler->show_progress_mode == ImageHandler::index_mode) {
      pdTitleStr = "Расчет индекса";   pdInfoStr = "Расчет индекса ..."; }
  progress_dialog =
          new QProgressDialog(pdInfoStr, "Отмена", 0, 100, this);
  progress_dialog->setFixedSize(350,100);
  progress_dialog->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
  progress_dialog->setWindowTitle(pdTitleStr);
  progress_dialog->resize(350, 100);
  progress_dialog->setMaximum(max_progress);
  progress_dialog->show();
  QApplication::processEvents();
  connect(im_handler, SIGNAL(reading_progress(int)), progress_dialog,
          SLOT(setValue(int)));
  connect(progress_dialog, SIGNAL(canceled()), this, SLOT(stop_reading_file()),
          Qt::DirectConnection);
}

void MainWindow::stop_reading_file() {
  im_handler->set_read_file_canceled();
  delete_progress_dialog();
  view->openAct->setEnabled(true);
  QApplication::processEvents();
}

void MainWindow::delete_progress_dialog() {
  if (progress_dialog != nullptr) {
    delete progress_dialog;
    progress_dialog = nullptr;
  }
}

void MainWindow::histogramSlot()
{
    imgHistogram->setVisible(!imgHistogram->isVisible());
}

void MainWindow::spectralSlot()
{
    auto z_list = view->getZGraphItemsList();  // список рафических объектов на сцене
    imgSpectral->updateData(dataFileName, z_list, true);
    imgSpectral->setVisible(!imgSpectral->isVisible());

}

void MainWindow::createDockWidgetForChannels()
{
    chListWidget->clear();
    auto strlist = im_handler->current_image()->get_wl_list(2);  // две цифры после запятой
    auto wavelengths = im_handler->current_image()->wls();
    foreach(QString str, strlist) {
        QListWidgetItem *lwItem = new QListWidgetItem();
        lwItem->setText(str);
        chListWidget->addItem(lwItem);
        int lwnum = strlist.indexOf(str);
        qreal wlen = wavelengths[lwnum];
        if (wlen < 781) {  // видимый диапазон
            QPixmap pixmap(16,16);
            QColor color = view->waveLengthToRGB(wlen);
            pixmap.fill(color);
            QIcon icon(pixmap);
            lwItem->setIcon(icon);
        } else
            lwItem->setIcon(QIcon(":/icons/color_NIR_32.png"));
     }  // for
    chListWidget->setCurrentRow(0);
}

void MainWindow::create_default_RGB_image()
{
// 84 - 641nm 53 - 550nm 22 - 460nm
    int num_red = im_handler->get_band_by_wave_length(rgb_default.red);
    int num_green = im_handler->get_band_by_wave_length(rgb_default.green);
    int num_blue = im_handler->get_band_by_wave_length(rgb_default.blue);
    if (num_red == -1 || num_green == -1 || num_blue == -1) {
        qDebug() << "CRITICAL! WRONG CONSTRUCTING THE RGB IMAGE";
        return;
    }  // if
    QImage img = im_handler->current_image()->get_rgb(true,num_red,num_green,num_blue);
    QString rgb_str = QString("R: %1 нм G: %2 нм B: %3 нм").arg(rgb_default.red)
            .arg(rgb_default.green).arg(rgb_default.blue);
    im_handler->current_image()->append_additional_image(img,rgb_str,rgb_str);
    uint32_t depth = im_handler->current_image()->get_bands_count();
    view->GlobalChannelNum = depth;
    im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
}

void MainWindow::restoreIndexListWidget() {
    int count = im_handler->current_image()->get_image_count();
    uint32_t depth = im_handler->current_image()->get_bands_count();
    for (int i = 0; i < count; i++) {
        im_handler->current_image()->set_current_slice(depth + i);
        QListWidgetItem *lwItem = new QListWidgetItem();
        indexListWidget->addItem(lwItem);
        lwItem->setIcon(QIcon(":/icons/palette.png"));
        QPair<QString, QString> formula = im_handler->current_image()->get_current_formula();
        lwItem->setText(formula.first);  lwItem->setToolTip(formula.second);
    }  // for
    view->GlobalChannelNum = depth;
    im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
}

void MainWindow::createDockWidgetForIndexes()
{
    indexListWidget->clear();  slider->setEnabled(true);
    if (restoreSettingAtStartUp) restoreIndexListWidget();
    if (indexListWidget->count() == 0) {
        create_default_RGB_image();
        QListWidgetItem *lwItem = new QListWidgetItem();
        indexListWidget->addItem(lwItem);
        lwItem->setIcon(QIcon(":/icons/palette.png"));
        QPair<QString, QString> formula = im_handler->current_image()->get_current_formula();
        lwItem->setText(formula.first);  lwItem->setToolTip(formula.second);
    }  // if
    indexListWidget->setCurrentRow(0);
    chListWidget->currentItem()->setSelected(false);  // конкурент
    set_abstract_index_pixmap();

}

QPointF MainWindow::getPointFromStr(QString str) {
    QStringList pos = str.split(",");
    return QPointF(pos[0].toDouble(),pos[1].toDouble());
}

QPoint MainWindow::getPoint__fromStr(QString str) {
    QStringList pos = str.split(",");
    return QPoint(pos[0].toInt(),pos[1].toInt());
}

QVector<double> MainWindow::getVectorFromStr(QString str) {
    QStringList strlist = str.split(",");
    QVector<double> v;
    foreach(QString s, strlist) v.append(s.toDouble());
    return v;
}

void MainWindow::restoreIndexes()
{
    bool SETTINGS_SET_EXISTS = im_handler->current_image()->load_formulas(save_formulas);
// при отсутствии ОДНОГО файла из набора игнорируем ВЕСЬ набор
    if (!SETTINGS_SET_EXISTS) return;
    im_handler->current_image()->load_additional_slices(save_slices);
    im_handler->current_image()->load_images(save_images);
    im_handler->current_image()->load_brightness(save_brightness);
}

void MainWindow::restoreSettings()
{
    if (dataFileName.isEmpty()) return;
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(dataFileName);
    QString iniFileName = writableLocation + '/' + info.completeBaseName()
            + "/" + info.completeBaseName() + ".ini";
    QFileInfo infoINI(iniFileName);
    if (!infoINI.exists()) return;
    QSettings settings( iniFileName, QSettings::IniFormat );
    QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
    settings.setIniCodec( codec );
    int ver = settings.value("version").toInt();
    switch (ver) {
    case 1 : { restoreSettingsVersionOne(settings); break; }
    case 2 : { restoreSettingsVersionTwo(settings); break ; }
    default: return;
    }  // switch
}

void MainWindow::loadMainSettings()
{

    QString fname = getMainSettingsFileName();
    if (!QFile::exists(fname)) {
        QFileInfo info(fname);
        QFile::copy("../BPLA/configuration/" + info.completeBaseName() + ".conf", fname);
    }  // if exists

    QFileInfo info(fname);
    QSettings settings( fname , QSettings::IniFormat );
    QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
    settings.setIniCodec( codec );

    settings.beginGroup( "General" );
    GLOBAL_SETTINGS.load_resent_project = settings.value( "load_resent_project", true).toBool();
    GLOBAL_SETTINGS.save_project_settings = settings.value( "save_project_settings", false).toBool();
    GLOBAL_SETTINGS.restore_project_settings = settings.value( "restore_project_settings", false).toBool();
    GLOBAL_SETTINGS.resent_files_count = settings.value( "resent_files_count", 3).toInt();
    GLOBAL_SETTINGS.save_not_checked_roi = settings.value( "save_not_checked_roi", true).toBool();
    GLOBAL_SETTINGS.zobject_dock_size_w = settings.value( "zobject_dock_size_w", 420).toInt();
    GLOBAL_SETTINGS.zobject_dock_size_h = settings.value( "zobject_dock_size_h", 150).toInt();
    GLOBAL_SETTINGS.zobjects_prof_rainbow_show = settings.value( "zobjects_prof_rainbow_show", true).toBool();
    settings.endGroup();

// СПИСОК СПЕКТРАЛЬНЫХ ИНДЕКСОВ
    settings.beginGroup( "Indexes" );           // Indexes
    QStringList list = settings.allKeys();
    foreach(QString str, list) {
        J09::indexType indexItem;
        indexItem.name = str;
        indexItem.formula = settings.value(str, "wrong").toString().toLower();
        if (indexItem.formula == "wrong") continue;
        indexList.append(indexItem);
    }  // foreach
    settings.endGroup();

}

void MainWindow::saveMainSettings()
{
    QString fname = getMainSettingsFileName();
    QSettings settings( fname , QSettings::IniFormat );
    QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
    settings.setIniCodec( codec );

    settings.beginGroup( "General" );
    settings.setValue( "load_resent_project", GLOBAL_SETTINGS.load_resent_project );
    settings.setValue( "save_project_settings", GLOBAL_SETTINGS.save_project_settings );
    settings.setValue( "restore_project_settings", GLOBAL_SETTINGS.restore_project_settings );
    settings.setValue( "resent_files_count", GLOBAL_SETTINGS.resent_files_count );
    settings.setValue( "save_not_checked_roi", GLOBAL_SETTINGS.save_not_checked_roi );
    settings.setValue( "zobject_dock_size_w", GLOBAL_SETTINGS.zobject_dock_size_w );
    settings.setValue( "zobject_dock_size_h", GLOBAL_SETTINGS.zobject_dock_size_h );
    settings.setValue( "zobjects_prof_rainbow_show", GLOBAL_SETTINGS.zobjects_prof_rainbow_show );
    settings.endGroup();

    settings.beginGroup( "Indexes" );
    foreach(J09::indexType item, indexList) settings.setValue( item.name, item.formula );
    settings.endGroup();

}

QString MainWindow::getMainSettingsFileName()
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(writableLocation);
    return writableLocation + "/" + info.completeBaseName() + ".conf";
}

void MainWindow::restoreSettingsVersionOne(QSettings &settings)
{
    view->GlobalScale = settings.value("scale", 1.0).toDouble();
    view->GlobalRotate = settings.value("rotation", 0.0).toDouble();
    view->GlobalChannelNum = settings.value("channelNum", 0).toInt();
    uint32_t depth = im_handler->current_image()->get_bands_count();
    if (view->GlobalChannelNum < depth) {
        QListWidgetItem *lwItem = chListWidget->item(view->GlobalChannelNum);
        itemClickedChannelList(lwItem);
        chListWidget->setCurrentItem(lwItem);  chListWidget->scrollToItem(lwItem);
    }  else {
        QListWidgetItem *lwItem = indexListWidget->item(view->GlobalChannelNum - depth);
        itemClickedIndexList(lwItem);
        indexListWidget->setCurrentItem(lwItem);  indexListWidget->scrollToItem(lwItem);
    }  // if
    view->GlobalChannelStep = settings.value("channelStep", 1).toInt();
    show_channel_list_update();

    int horizontal = settings.value("horizontal", 0).toInt();
    int vertical = settings.value("vertical", 0).toInt();
    view->resetMatrix();  view->resetTransform();
    view->scale(1/view->GlobalScale, 1/view->GlobalScale);
    view->rotate(-view->GlobalRotate);
    view->horizontalScrollBar()->setValue(horizontal);
    view->verticalScrollBar()->setValue(vertical);
    auto graphlist = settings.value("dockZGraphList", true).toBool();
    auto channellist = settings.value("dockChannelList", true).toBool();
    auto indexlist = settings.value("dockIndexList", true).toBool();
    dockZGraphList->setVisible(graphlist);
    dockChannelList->setVisible(channellist);
    dockIndexList->setVisible(indexlist);
    view->winZGraphListAct->setChecked(dockZGraphList->isVisible());
    view->channelListAct->setChecked(dockChannelList->isVisible());
    view->indexListAct->setChecked(dockIndexList->isVisible());
    view->PAN = false;
    QStringList groups = settings.childGroups();

    createZGraphItemFromGroups(groups, settings);  // создать объекты из текстового списка

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray(), 1);
    restoreTRUEdockWidgetsPosition();  // restoreGeometry !!! путает последовательность окон профилей
}

void MainWindow::createZGraphItemFromGroups(QStringList &groups, QSettings &settings)
{
    foreach(QString str, groups) {
        if (str.indexOf("Point") != -1) {
            settings.beginGroup(str);
            QString pos = settings.value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zPoint *point = new zPoint( pXY );
            QString title = settings.value("title").toString();
            point->setTitle(title);
            point->aicon = QIcon(":/images/pin_grey.png");
            bool objectvisible = settings.value("objectvisible", true).toBool();
            point->setVisible(objectvisible);
// create dynamic DockWidget
            view->scene()->addItem(point);          // view->scene()->addItem(point);
            point->updateBoundingRect();
            emit view->insertZGraphItem(point);
            view->show_profile_for_Z(point);
            emit view->setZGraphDockToggled(point);
            point->dockw->setVisible(settings.value("dockvisible", true).toBool());
            QString dockwpos = settings.value("dockwpos").toString();
            point->dockwpos = getPoint__fromStr(dockwpos);
            settings.endGroup();
// create dynamic DockWidget
            continue;
        }  // Point
        if (str.indexOf("Rect") != -1) {
            settings.beginGroup(str);
            QString pos = settings.value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
//            QVector<double> v = getVectorFromStr(pos);
//            QPointF pXY(v.at(0), v.at(1));
            zRect *rect = new zRect(pXY,view->GlobalScale,view->GlobalRotate);
            QString title = settings.value("title").toString();
            rect->setTitle(title);
            rect->aicon = QIcon(":/images/vector_square.png");
            bool objectvisible = settings.value("objectvisible", true).toBool();
            rect->setVisible(objectvisible);
            double rotation = settings.value("rotation").toDouble();
//            qreal rotation = rect->getRotationFromCoords( QPointF(v.at(0),v.at(1)), QPointF(v.at(2),v.at(3)) );
            rect->setRotation(rotation);
            QString sizestr = settings.value("size").toString();
            QPointF fsize = getPointFromStr(sizestr);
            rect->frectSize.setWidth(fsize.rx());
            rect->frectSize.setHeight(fsize.ry());
// create dynamic DockWidget
            view->scene()->addItem(rect);          // view->scene()->addItem(rect);
            rect->updateBoundingRect();
            emit view->insertZGraphItem(rect);
            view->show_profile_for_Z(rect);
            emit view->setZGraphDockToggled(rect);
            rect->dockw->setVisible(settings.value("dockvisible", true).toBool());
            QString dockwpos = settings.value("dockwpos").toString();
            rect->dockwpos = getPoint__fromStr(dockwpos);
            settings.endGroup();
// create dynamic DockWidget
            continue;
        }  // Rect
        if (str.indexOf("Ellipse") != -1) {
            settings.beginGroup(str);
            QString pos = settings.value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
//            QVector<double> v = getVectorFromStr(pos);
//            QPointF pXY(v.at(0), v.at(1));
            zEllipse *ellipse = new zEllipse(pXY,view->GlobalScale,view->GlobalRotate);
            QString title = settings.value("title").toString();
            ellipse->setTitle(title);
            ellipse->aicon = QIcon(":/images/vector_ellipse.png");
            bool objectvisible = settings.value("objectvisible", true).toBool();
            ellipse->setVisible(objectvisible);
            double rotation = settings.value("rotation").toDouble();
//            qreal rotation = ellipse->getRotationFromCoords( QPointF(v.at(0),v.at(1)), QPointF(v.at(2),v.at(3)) );
            ellipse->setRotation(rotation);
            QString sizestr = settings.value("size").toString();
            QPointF fsize = getPointFromStr(sizestr);
            ellipse->frectSize.setWidth(fsize.rx());
            ellipse->frectSize.setHeight(fsize.ry());
// create dynamic DockWidget
            view->scene()->addItem(ellipse);          // view->scene()->addItem(ellipse);
            ellipse->updateBoundingRect();
            emit view->insertZGraphItem(ellipse);
            view->show_profile_for_Z(ellipse);
            emit view->setZGraphDockToggled(ellipse);
            ellipse->dockw->setVisible(settings.value("dockvisible", true).toBool());
            QString dockwpos = settings.value("dockwpos").toString();
            ellipse->dockwpos = getPoint__fromStr(dockwpos);
            settings.endGroup();
// create dynamic DockWidget
            continue;
        }  // Ellipse
        if (str.indexOf("Polyline") != -1) {
            settings.beginGroup(str);
            QString pos = settings.value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zPolyline *polyline = new zPolyline( view->GlobalScale,view->GlobalRotate );
            QString title = settings.value("title").toString();
            polyline->setTitle(title);
            polyline->setPos(pXY);
            polyline->aicon = QIcon(":/images/polyline-64.png");
            bool objectvisible = settings.value("objectvisible", true).toBool();
            polyline->setVisible(objectvisible);
            QString xstr = settings.value("x").toString();
            QString ystr = settings.value("y").toString();
            auto x = getVectorFromStr(xstr);
            auto y = getVectorFromStr(ystr);
            for (int i=0; i<x.length();i++)
                polyline->fpolygon.append(QPoint(x[i],y[i]));
// create dynamic DockWidget
            view->scene()->addItem(polyline);          // view->scene()->addItem(polyline);
            polyline->updateBoundingRect();
            emit view->insertZGraphItem(polyline);
            view->show_profile_for_Z(polyline);
            emit view->setZGraphDockToggled(polyline);
            polyline->dockw->setVisible(settings.value("dockvisible", true).toBool());
            QString dockwpos = settings.value("dockwpos").toString();
            polyline->dockwpos = getPoint__fromStr(dockwpos);
            settings.endGroup();
// create dynamic DockWidget
            continue;
        }  // Polyline
        if (str.indexOf("Polygon") != -1) {
            settings.beginGroup(str);
            QString pos = settings.value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zPolygon *polygon = new zPolygon( view->GlobalScale,view->GlobalRotate );
            QString title = settings.value("title").toString();
            polygon->setTitle(title);
            polygon->setPos(pXY);
            polygon->aicon = QIcon(":/images/vector-polygon.png");
            bool objectvisible = settings.value("objectvisible", true).toBool();
            polygon->setVisible(objectvisible);
            QString xstr = settings.value("x").toString();
            QString ystr = settings.value("y").toString();
            auto x = getVectorFromStr(xstr);
            auto y = getVectorFromStr(ystr);
            for (int i=0; i<x.length();i++)
                polygon->fpolygon.append(QPoint(x[i],y[i]));
// create dynamic DockWidget
            view->scene()->addItem(polygon);          // view->scene()->addItem(polygon);
            polygon->updateBoundingRect();
            emit view->insertZGraphItem(polygon);
            view->show_profile_for_Z(polygon);
            emit view->setZGraphDockToggled(polygon);
            polygon->dockw->setVisible(settings.value("dockvisible", true).toBool());
            QString dockwpos = settings.value("dockwpos").toString();
            polygon->dockwpos = getPoint__fromStr(dockwpos);
            settings.endGroup();
// create dynamic DockWidget
            continue;
        }  // Polygon
    }  // foreach

}

void MainWindow::restoreTRUEdockWidgetsPosition()
{
    auto graphList = view->getZGraphItemsList();
    foreach(zGraph *item, graphList) {

        item->dockw->move(item->dockwpos - QPoint(76-68,43-12));
        item->dockw->setAllowedAreas(Qt::NoDockWidgetArea);
        item->plot->rescaleAxes();
        item->plot->yAxis->setRangeLower(.0);
        item->plot->replot();

    }  // foreach
}

void MainWindow::restoreDockWidgetsPositionExt()
{
    auto graphList = view->getZGraphItemsList();
    foreach(zGraph *item, graphList) {

        item->plot->rescaleAxes();
        item->plot->yAxis->setRangeLower(.0);
        item->plot->replot();
        QPoint item_pos = view->mapFromScene(item->pos().rx(), item->pos().ry());
        item_pos = this->mapToGlobal(item_pos);
        if (item_pos.x() < 50) continue;
        if (item_pos.y() < 50) continue;
        QRect rect = QApplication::desktop()->screenGeometry();
        if (item_pos.x() > rect.width() - 70) continue;
        if (item_pos.y() > rect.height() - 70) continue;
        item->dockw->move(item_pos + QPoint(0,60));
        item->dockw->setAllowedAreas(Qt::NoDockWidgetArea);

    }  // foreach
}

void MainWindow::restoreSettingsVersionTwo(QSettings &settings)
{
    Q_UNUSED(settings)
}

void MainWindow::itemClickedChannelList(QListWidgetItem *lwItem)
{
    indexListWidget->currentItem()->setSelected(false);  // конкурент
    int num = chListWidget->row(lwItem);
    view->GlobalChannelNum = num;
    im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
//----------------------------------------------------------------
    set_abstract_index_pixmap();
    imgHistogram->hide();  histogramAct->setEnabled(false);
}

void MainWindow::itemClickedIndexList(QListWidgetItem *lwItem)
{
    chListWidget->currentItem()->setSelected(false);  // конкурент
    int num = indexListWidget->row(lwItem);
    uint32_t depth = im_handler->current_image()->get_bands_count();
    view->GlobalChannelNum = num + depth;
    im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
//----------------------------------------------------------------
    set_abstract_index_pixmap();
    if (num==0) {imgHistogram->hide(); histogramAct->setEnabled(false); }
    else { updateHistogram(); histogramAct->setEnabled(true); }
}

void MainWindow::addRecentFile()
{
    if (recentFileNames.indexOf(dataFileName) != -1) return;
    QAction *recentAct = new QAction(this);
    QString fname = im_handler->current_image()->get_file_name();
    QFileInfo info(fname);
    recentAct->setText(info.fileName());
    recentAct->setData(fname);
    recentAct->setIcon(QIcon(view->mainPixmap->pixmap()));
    fileToolBar->addAction(recentAct);
    connect(recentAct, SIGNAL(triggered()), this, SLOT(OpenRecentFile()));
    recentFileNames.append(dataFileName);
    fileMenu->insertAction(addSeparatorRecentFile, recentAct);
}

void MainWindow::saveGLOBALSettings()
{
    QString iniFileName = getGlobalIniFileName();
    QSettings settings( iniFileName, QSettings::IniFormat );
    QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
    settings.setIniCodec( codec );
    settings.clear();

}

QString MainWindow::getGlobalIniFileName()
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(writableLocation);
    return writableLocation + "/" + info.completeBaseName() + ".ini";
}

void MainWindow::OpenRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        if (dataFileName == action->data().toString()) return;
        QStringList fnames = im_handler->get_image_file_names();
        int num = fnames.indexOf(action->data().toString());
        if (num == -1) {
            dataFileName = action->data().toString();
            emit read_file(dataFileName);
// connect(this, SIGNAL(read_file(QString)),im_handler,SLOT(read_envi_hdr(QString)), Qt::DirectConnection);
        } else {
// сохранияем конфигурацию перед переключениеь на другой набор данных
            saveSettings();
            auto set_current = im_handler->set_current_image(num);
            if (!set_current) return;
            dataFileName = im_handler->current_image()->get_file_name();
            updateNewDataSet(false);
        }  // if (num == -1)
    }  // if (action)
}

void MainWindow::showContextMenuChannelList(const QPoint &pos)
{
    QPoint globalPos = chListWidget->mapToGlobal(pos);
    QMenu menu(this);
    for (int i = 0; i < show_channel_list_acts.count(); i++) {
        menu.addAction(show_channel_list_acts[i]);
        if (i == 0 ) menu.addSeparator();
    }  // for
    menu.exec(globalPos);
}

void MainWindow::showContextMenuZGraphList(const QPoint &pos)
{
    QPoint globalPos = zGraphListWidget->mapToGlobal(pos);
    QMenu menu(this);
    for (int i = 0; i < show_zgraph_list_acts.count(); i++)
        menu.addAction(show_zgraph_list_acts[i]);

    QMenu *maskMenu = menu.addMenu(tr("Отфильтровать по маске"));

    for (int i = 0; i < mask_list_acts.count(); i++)
        maskMenu->addAction(mask_list_acts[i]);

    menu.exec(globalPos);
}

void MainWindow::showContextMenuDockIndexList(const QPoint &pos)
{
    if (dataFileName.isEmpty()) return;
    QPoint globalPos = indexListWidget->mapToGlobal(pos);
    QMenu menu(this);
    for (int i = 0; i < show_index_list_acts.count(); i++) {
        if (i == 0) {
            QString current_item = indexListWidget->currentItem()->text();
            show_index_list_acts[i]->setText(QString("Удалить индексное изображение %1").arg(current_item));
        }  // if
        menu.addAction(show_index_list_acts[i]);
    }  // for
    menu.exec(globalPos);
}

void MainWindow::show_index_list()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        int num = action->data().toInt();
        switch (num) {
        case 0 : {    // Удалить индексное изображение
            QMessageBox msgBox;
            msgBox.setWindowTitle("Удаление индексного изображения");
            msgBox.setText(QString("Вы подтверждаете удаление индексного изображения %1 ?\nВосстановить удаление будет невозможно !")
                           .arg(indexListWidget->currentItem()->text()));
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.buttons().at(0)->setText("Да");
            msgBox.buttons().at(1)->setText("Отмена");
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowIcon(icon256);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            QPoint pos = dockIndexList->pos();
            QPoint mv(-300,200);
            msgBox.move(pos + mv);

            int res = msgBox.exec();

     /*       if (res == QMessageBox::Ok) {   // нажата кнопка Да
                view->clearForAllObjects();
                zGraphListWidget->clear();
                spectralAct->setEnabled(false);
                imgSpectral->hide();
            }  // if QMessageBox::Ok */

            return;
        }  // case 0
        }  // switch
    }  // if
}

void MainWindow::show_channel_list_update() {
    for (int row = 0; row < chListWidget->count(); row++) {
        if (row % view->GlobalChannelStep == 0)
            chListWidget->item(row)->setHidden(false);
        else chListWidget->item(row)->setHidden(true);
    }  // for
}

void MainWindow::show_channel_list()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        view->GlobalChannelStep = action->data().toInt();
        show_channel_list_update();
    }  // if
}

void MainWindow::show_zgraph_list()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        int num = action->data().toInt();
        auto zGraphList = view->getZGraphItemsList();
        switch (num) {
        case 1 : {    // Отобразить все области интереса
            foreach(zGraph *item, zGraphList) {
                item->setVisible(true);
                item->listwidget->setCheckState(Qt::Checked);
            }  // foreach
            return;
        }  // case 1
        case 2 : {    // Скрыть все области интереса
            foreach(zGraph *item, zGraphList) {
                item->setVisible(false);
                item->listwidget->setCheckState(Qt::Unchecked);
            }  // foreach
            return;
        }  // case 2
        case 3 : {    // Отобразить все профили
            foreach(zGraph *item, zGraphList) {
                item->dockw->setVisible(true);
                QFont font = item->listwidget->font();  font.setBold(item->dockw->isVisible());
                item->listwidget->setFont(font);
            }  // foreach
            return;
        }  // case 3
        case 4 : {    // Скрыть все профили
            foreach(zGraph *item, zGraphList) {
                item->dockw->setVisible(false);
                QFont font = item->listwidget->font();  font.setBold(item->dockw->isVisible());
                item->listwidget->setFont(font);
            }  // foreach
            return;
        }  // case 4
        case 5 : {    // Сохранить все профили в Excel *.csv файл ...
            if (zGraphList.count() == 0) return;
            QString csv_file_name = QFileDialog::getSaveFileName(
                this, tr("Сохранить профили в Excel *.csv файл"), QCoreApplication::applicationDirPath(),
                tr("CSV Files Excel (*.csv);;CSV Files (*.scv)"));
            if (csv_file_name.isEmpty()) return;
            QStringList csv_list;
            QString str = "длина волны, нм;";
            foreach(zGraph *item, zGraphList) str.append(item->getTitle() + ";");
            csv_list.append(str);
            int count = zGraphList[0]->profile.count();
            for (int i=0; i<count; i++) {
                QString str = QString(tr("%1;").arg(zGraphList[0]->profile[i].rx()));
                foreach(zGraph *item, zGraphList)
                    str.append(QString(tr("%1;").arg(item->profile[i].ry())));
                str = str.replace('.', ',');
                csv_list.append(str);
            }  // for i
            QFile file(csv_file_name);
            file.open( QIODevice::Append | QIODevice::Text );
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            stream.setGenerateByteOrderMark(true);
            foreach(QString str, csv_list) stream << str << endl;
            stream.flush();
            file.close();
            return;
        }  // case 5
        case 6 : {    // Загрузить области интереса из файла ...
            auto proj_path = getDataSetPath();
            if (proj_path.isEmpty()) {
                QMessageBox msgBox;
                msgBox.setWindowIcon(icon256);
                msgBox.setWindowTitle("Ошибка");
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(" Нет загруженного проекта !");
                msgBox.exec();
                return;
            }
            QFileDialog fd;
            fd.move(dockZGraphList->pos() + QPoint(-fd.width(),dockZGraphList->height()));
            QString fname = fd.getOpenFileName(
                this, tr("Загрузка выделенных областей интереса"), proj_path,
                tr("Файлы областей интереса (*.roi);;Файлы областей интереса (*.roi)"));
            if (fname.isEmpty()) return;

            QSettings settings( fname, QSettings::IniFormat );
            QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
            settings.setIniCodec( codec );
            QStringList groups = settings.childGroups();

            createZGraphItemFromGroups(groups, settings);  // создать объекты из текстового списка
            restoreDockWidgetsPositionExt();  // восстанавливает коррдинаты окон для загруженных ОИ
            auto graphList = view->getZGraphItemsList();
            if (imgSpectral->isVisible()) imgSpectral->updateData(dataFileName, graphList, true);  // обновление всех профилей
            if (graphList.count() > 0) spectralAct->setEnabled(true);

            return;
        }  // case 6
        case 7 : {    // Сохранить выделенные области интереса в файл ...
            auto proj_path = getDataSetPath();
            if (proj_path.isEmpty()) {
                QMessageBox msgBox;
                msgBox.setWindowIcon(icon256);
                msgBox.setWindowTitle("Ошибка");
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(" Нет загруженного проекта !");
                msgBox.exec();
                return;
            }
// проверка на наличие сохраняемых областей интереса
            auto graphList = view->getZGraphItemsList();
            int count = 0;
            foreach(zGraph *item, graphList) if (item->isVisible()) count++;
            if (!count) {
                QMessageBox msgBox;
                msgBox.setWindowIcon(icon256);
                msgBox.setWindowTitle("Ошибка");
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(" Нет выделенных областей интереса !");
                msgBox.exec();
                return;
            }
            QString fname = QFileDialog::getSaveFileName(
                this, tr("Сохранение выделенных областей интереса"), proj_path,
                tr("Файлы областей интереса (*.roi);;Файлы областей интереса (*.roi)"));
            if (fname.isEmpty()) return;
            QSettings settings( fname, QSettings::IniFormat );
            QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
            settings.setIniCodec( codec );
            settings.clear();
            saveSettingsZ(settings, true);
            return;
        }  // case 7
        case 11 : {    // Удалить все области интереса ...
            QMessageBox msgBox;
            msgBox.setWindowTitle("Удаление всех областей интереса");
            msgBox.setText("Вы подтверждаете удаление всех областей интереса ?\nВосстановить удаление будет невозможно !");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.buttons().at(0)->setText("Да");
            msgBox.buttons().at(1)->setText("Отмена");
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowIcon(icon256);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            QPoint pos = dockZGraphList->pos();
            QPoint mv(-300,200);
            msgBox.move(pos + mv);

            int res = msgBox.exec();
            if (res == QMessageBox::Ok) {   // нажата кнопка Да
                view->clearForAllObjects();
                zGraphListWidget->clear();
                spectralAct->setEnabled(false);
                imgSpectral->hide();
            }  // if QMessageBox::Ok

            return;
        }  // case 11
        }  // switch
    }  // if
}

void MainWindow::inputIndexDlgShow()
{
    inputIndexDlg *dlg = new inputIndexDlg(this);
// update index list
    dlg->setDefaultIndexList(indexList);

    dlg->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
    dlg->ui->buttonBox->buttons().at(1)->setText("Отмена");
    QVector<double> wls = im_handler->current_image()->wls();
    dlg->setSpectralRange(wls);
    if (dlg->exec() == QDialog::Accepted) {
//        view->index_title_str = dlg->get_index_title();
//        view->index_formula_str = dlg->get_formula();
        calculateIndexAndStoreToIndexList(dlg->get_index_title(), dlg->get_formula());

        /*QString input = dlg->get_formula();
        if (input.isEmpty()) {
            qDebug() << "wrong index !!!";
            return;
        }  // if
        QString for_eval = im_handler->get_regular_expression(input);
        if (for_eval.isEmpty()) {
            qDebug() << "wrong index !!!";
            return;
        }  // if
// update index list
        J09::indexType indexItem;
        indexItem.name = dlg->ui->lineEditInputTitle->text();
        indexItem.formula = dlg->ui->lineEditInputFormula->text();
        if (dlg->namesList.indexOf(indexItem.name) == -1)
            indexList.append(indexItem);

        emit index_calculate(for_eval); */
    }  // if
}

void MainWindow::calculateIndexAndStoreToIndexList(QString title, QString formula)
{
    view->index_title_str = title;
    view->index_formula_str = formula;

    QString input = formula;
    if (input.isEmpty()) {
        qDebug() << "wrong index !!!";
        return;
    }  // if
    QString for_eval = im_handler->get_regular_expression(input);
    if (for_eval.isEmpty()) {
        qDebug() << "wrong index !!!";
        return;
    }  // if
    // update index list
    J09::indexType indexItem;
    indexItem.name = title;
    indexItem.formula = formula;
    QStringList str_list;
    foreach(J09::indexType item, indexList) str_list.append(item.name);
    if (str_list.indexOf(indexItem.name) == -1)
        indexList.append(indexItem);

    emit index_calculate(for_eval);
}

void MainWindow::predefined_index_menu()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QString title = action->text();
        QString formula = action->data().toString();
        qDebug() << title << formula;
        calculateIndexAndStoreToIndexList(title, formula);
    }  // if
}

void MainWindow::settingsDlgShow()
{
    settingsDialog *dlg = new settingsDialog(this);
    dlg->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
    dlg->ui->buttonBox->buttons().at(1)->setText("Отмена");
    if (dlg->exec() == QDialog::Accepted) {}
}

void MainWindow::change_brightness(int value)
{
    QPixmap pxm;
    QImage img = im_handler->get_REAL_current_image();
    double brightness = im_handler->get_new_brightness(slider, value);
    pxm = view->changeBrightnessPixmap(img, brightness);
    view->mainPixmap->setPixmap(pxm) ;
}

void MainWindow::add_index_pixmap(int num)
{
// num - номер максимального slice, этот номер на 1 меньше чем нужный current_slice
    view->GlobalChannelNum = num + 1;  histogramAct->setEnabled(true);
// ВНИМАНИЕ!!!  img SpectralImage отличается от indexImages тем , что под номером
//  depth в indexImages находится RGB, и оно отсутствуем в img SpectralImage
    QImage img = im_handler->current_image()->get_index_rgb(true,true,num);
    im_handler->current_image()->append_additional_image(img,
                                 view->index_title_str,view->index_formula_str);
    im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
    QListWidgetItem *lwItem = new QListWidgetItem();
    indexListWidget->addItem(lwItem);
    lwItem->setIcon(QIcon(":/icons/palette.png"));
    QPair<QString, QString> formula = im_handler->current_image()->get_current_formula();
    lwItem->setText(formula.first);  lwItem->setToolTip(formula.second);
    indexListWidget->setCurrentRow(indexListWidget->count() - 1);
    chListWidget->currentItem()->setSelected(false);  // конкурент
//    (r640-r680)/(640+r680)
//----------------------------------------------------------
    set_abstract_index_pixmap();
    show_histogram_widget();  // histogram
}

void MainWindow::set_abstract_index_pixmap()
{
    double brightness = im_handler->current_image()->get_current_brightness();
    im_handler->set_brightness_to_slider(slider, brightness);
    QImage img = im_handler->get_REAL_current_image();
    QPixmap pxm = view->changeBrightnessPixmap(img, brightness);
    view->mainPixmap->setPixmap(pxm);
}

void MainWindow::show_histogram_widget() {

    updateHistogram();
}

void MainWindow::updateHistogram()
{
    uint32_t depth = im_handler->current_image()->get_bands_count();
    double brightness = im_handler->current_image()->getBrightness(depth);  // RGB - slice
    QImage img = im_handler->current_image()->get_additional_image(0);   // RGB - image
    maiPixmap = view->changeBrightnessPixmap(img, brightness);

    QVector<QVector<double> > slice_index = im_handler->current_image()->get_band(view->GlobalChannelNum-1);
    QPair<QString, QString> name = im_handler->current_image()->get_current_formula();

    imgHistogram->setPreviewPixmap(maiPixmap);
    imgHistogram->updateData(dataFileName, name.first, name.second, slice_index,
                             im_handler->current_image()->histogram[view->GlobalChannelNum-1]);
    if (histogramAct->isChecked()) imgHistogram->show();
    else imgHistogram->hide();
}

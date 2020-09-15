#include "mainwindow.h"
#include "zgraphparamdlg.h"
#include "ui_zgraphparamdlg.h"
#include "inputindexdlg.h"
#include "ui_inputindexdlg.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "saveflistdlg.h"
#include "ui_saveflistdlg.h"

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
  setWindowTitle(appName);
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
  createResentFilesList();

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
  fileToolBar = addToolBar(fileMenu->title());
  fileToolBar->setObjectName("fileToolBar");fileToolBar->setIconSize(tb_def_size);
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
    QToolBar *indexToolBar = addToolBar(indexMenu->title());
    indexToolBar->setObjectName("indexToolBar");indexToolBar->setIconSize(tb_def_size);
    const QIcon indexIcon =
        QIcon::fromTheme("Добавить индексное изображение", QIcon(":/icons/MapleLeaf.png"));
    indexAct = new QAction(indexIcon, tr("&Добавить индексное изображение"), this);
    indexMenu->addAction(indexAct);   indexToolBar->addAction(indexAct);
    connect(indexAct, SIGNAL(triggered()), this, SLOT(inputIndexDlgShow()));

    index_quick_menu = new QMenu(tr("&Быстрый выбор индекса"), this);
    index_quick_menu->setIcon(indexIcon);

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

    maskAct->setCheckable(true); maskAct->setChecked(false);
    maskAct->setEnabled(false);
    indexMenu->addAction(maskAct);   indexToolBar->addAction(maskAct);

// &&& windows
  QMenu *winMenu = menuBar()->addMenu("&Окна");
  QToolBar *winToolBar = new QToolBar(winMenu->title());
//  QToolBar *winToolBar = addToolBar(winMenu->title());
  addToolBar(Qt::LeftToolBarArea, winToolBar);

  winToolBar->setObjectName("winToolBar");winToolBar->setIconSize(tb_def_size);
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
  settingsToolBar->setObjectName("settingsToolBar");winToolBar->setIconSize(tb_def_size);
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

void MainWindow::imgMasksUpdatePreviewPixmap()
{
    uint32_t depth = im_handler->current_image()->get_bands_count();
    double brightness = im_handler->current_image()->get_index_brightness(0);  // RGB - slice
    QImage img = im_handler->current_image()->get_additional_image(0);   // RGB - image
    mainPixmap = view->changeBrightnessPixmap(img, brightness);
    imgMasks->setPreviewPixmap(mainPixmap);
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

    connect(maskAct, SIGNAL(triggered()), this, SLOT(masksSlot()));  // маски
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
// masks

}

void MainWindow::createResentFilesList()
{
    QString fname = getMainSettingsFileName();
    QSettings settings( fname , QSettings::IniFormat );
    QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
    settings.setIniCodec( codec );
    settings.beginGroup( "Resent files" );
    const QStringList childKeys = settings.childKeys();

    recentFileNames.clear();
    view->resentFilesActions.clear();
    foreach (const QString &childKey, childKeys) {
        QString fname = settings.value(childKey).toString();
        QFileInfo info(fname);
        QIcon icon = im_handler->load_icon_from_file(fname, GLOBAL_SETTINGS.main_rgb_rotate_start);
        QAction *recentAct = new QAction(this);
        recentAct->setText(info.fileName());
        recentAct->setData(fname);
        recentAct->setIcon(icon);
        fileToolBar->addAction(recentAct);
        connect(recentAct, SIGNAL(triggered()), this, SLOT(OpenRecentFile()));
        recentFileNames.append(fname);
        fileMenu->insertAction(addSeparatorRecentFile, recentAct);
        view->resentFilesActions.append(recentAct);
    }

    settings.endGroup();

}

void MainWindow::spectralUpdateExt(zGraph *item)
{
    if (item != nullptr) item->data_file_name = dataFileName;
    if (!imgSpectral->isVisible()) return;
    auto zz_list = view->getZGraphItemsList();

    imgSpectral->updateDataExt(dataFileName, zz_list, item);

}

void MainWindow::createConstDockWidgets()
{
    QRect rec = QApplication::desktop()->screenGeometry();
    int scrHeignt4 = qRound(rec.height() / 4.5);
// ОБЛАСТИ ИНТЕРЕСА
    dockZGraphList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(dockZGraphList, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenuZGraphList(QPoint)));
    dockZGraphList->setWindowIcon(QIcon(":/icons/windows2.png"));
    dockZGraphList->setFloating(true);  dockZGraphList->setObjectName("dockZGraphList");
    dockZGraphList->setFixedSize(220,scrHeignt4-36);
    dockZGraphList->setWidget(zGraphListWidget);
    int upperIndent = 30;
    dockZGraphList->move(rec.width() - 250,upperIndent+0*scrHeignt4);
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

// ИЗОБРАЖЕНИЯ
    dockIndexList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(dockIndexList, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenuDockIndexList(QPoint)));
    dockIndexList->setWindowIcon(QIcon(":/icons/palette.png"));
    dockIndexList->setFloating(true);  dockIndexList->setObjectName("dockIndexList");
    dockIndexList->setFixedSize(220,scrHeignt4-36);
    dockIndexList->setWidget(indexListWidget);
    indexListWidget->setIconSize(indexIconSize);
    dockIndexList->move(rec.width() - 250,upperIndent+1*scrHeignt4);
    addDockWidget(Qt::BottomDockWidgetArea, dockIndexList);
    connect(indexListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClickedIndexList(QListWidgetItem*)));
    connect(dockIndexList->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
// контекстное меню для индексных изображений
    show_index_list_acts.append(histogramAct);

    createContextAction(QIcon(":/icons/ok.png"), "Выделить все индексные изображения", 7,  // check all
                        show_index_list_acts, SLOT(show_index_list()));
    createContextAction(QIcon(":/icons/unchecked-checkbox.png"), "Снять выделение со всех индексных изображений", 8,  // uncheck all
                        show_index_list_acts, SLOT(show_index_list()));
    action = new QAction(this);  action->setSeparator(true);  show_index_list_acts.append(action);

    createContextAction(QIcon(":/icons/rainbow_icon_cartoon_style.png"), "Сохранить выделенные индексные изображения  в *.index файл ...", 1,  // save checked
                        show_index_list_acts, SLOT(show_index_list()));
    createContextAction(QIcon(":/icons/rainbow_icon_cartoon_style.png"), "Загрузить индексные изображения из *.index файла ...", 2,  // load
                        show_index_list_acts, SLOT(show_index_list()));

    createContextAction(QIcon(":/icons/save.png"), "Сохранить выделенные индексы в файлы ...", 3,  // save checked pdf
                        show_index_list_acts, SLOT(show_index_list()));
//    createContextAction(QIcon(":/icons/csv2.png"), "Сохранить выделенные индексы в Excel CSV файлы ...", 4,  // save checked csv
//                        show_index_list_acts, SLOT(show_index_list()));
    action = new QAction(this);  action->setSeparator(true);  show_index_list_acts.append(action);
    createContextAction(QIcon(":/icons/delete-32.png"), "Удалить ВСЕ индексные изображения (кроме RGB) ...", 5,  // delete ALL (rgb)
                        show_index_list_acts, SLOT(show_index_list()));
    createContextAction(QIcon(":/icons/delete-32-1.png"), "Удалить выделенные индексные изображения (кроме RGB) ...", 6,  // delete checked (rgb)
                        show_index_list_acts, SLOT(show_index_list()));

// СПИСОК КАНАЛОВ
    int correct_ch_mask = 10;
    chListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(chListWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenuChannelList(QPoint)));
    dockChannelList->setWindowIcon(QIcon(":/icons/list-channel.jpg"));
    dockChannelList->setFloating(true);  dockChannelList->setObjectName("dockChannelList");
    dockChannelList->setFixedSize(220,scrHeignt4-36-correct_ch_mask);
    dockChannelList->setWidget(chListWidget);
    dockChannelList->move(rec.width() - 250,upperIndent+2*scrHeignt4);
    addDockWidget(Qt::BottomDockWidgetArea, dockChannelList);
    connect(chListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClickedChannelList(QListWidgetItem*)));
    connect(dockChannelList->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
// контекстное меню для отображения части каналов с шагом 2-10
    action = new QAction(QIcon(":/icons/all_channels.png"), "Отображать все каналы", this);
    action->setData(1);  show_channel_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_channel_list()));

    createContextAction(QIcon(":/icons/ok.png"), "Выделить все каналы", 11,  // check all
                        show_channel_list_acts, SLOT(show_channel_list()));
    createContextAction(QIcon(":/icons/unchecked-checkbox.png"), "Снять выделение со всех каналов", 12,  // uncheck all
                        show_channel_list_acts, SLOT(show_channel_list()));
    createContextAction(QIcon(":/icons/open.png"), "Загрузить и отобразить список каналов ...", 17,  // load checked
                        show_channel_list_acts, SLOT(show_channel_list()));
    createContextAction(QIcon(":/icons/save.png"), "Сохранить список выделенных каналов ...", 18,  // save checked
                        show_channel_list_acts, SLOT(show_channel_list()));
    action = new QAction(this);  action->setSeparator(true);  show_channel_list_acts.append(action);

    createContextAction(QIcon(":/icons/ok.png"), "Отображать только выделенные каналы", 14,  // check only show
                        show_channel_list_acts, SLOT(show_channel_list()));
    action = new QAction(QIcon(":/icons/step2_channels.png"), "Отображать каналы с шагом 2", this);
    action->setData(2);  show_channel_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_channel_list()));
    action = new QAction(QIcon(":/icons/step5_channels.png"), "Отображать каналы с шагом 5", this);
    action->setData(5);  show_channel_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_channel_list()));
    action = new QAction(QIcon(":/icons/step10_channels.png"), "Отображать каналы с шагом 10", this);
    action->setData(10);  show_channel_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(show_channel_list()));

    action = new QAction(this);  action->setSeparator(true);  show_channel_list_acts.append(action);
    createContextAction(QIcon(":/icons/save.png"), "Сохранить выделенные каналы в файлы ...", 15,  // save checked pdf png jpg jpeg csv
                        show_channel_list_acts, SLOT(show_channel_list()));
//    createContextAction(QIcon(":/icons/csv2.png"), "Сохранить выделенные каналы в Excel CSV файлы ...", 16,  // save checked csv
//                        show_channel_list_acts, SLOT(show_channel_list()));


// СПЕКТРАЛЬНЫЙ АНАЛИЗ
    imgSpectral->hide();
    addDockWidget(Qt::BottomDockWidgetArea, imgSpectral);
    connect(imgSpectral->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
    imgSpectral->gsettings = &GLOBAL_SETTINGS;  imgSpectral->setDefaultState();
    imgSpectral->imgHand = im_handler;

// АНАЛИЗ ИНДЕКСНЫХ ИЗОБРАЖЕНИЙ
    imgHistogram->hide();
    addDockWidget(Qt::BottomDockWidgetArea, imgHistogram);
    connect(imgHistogram->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
    connect(imgHistogram, &imageHistogram::appendMask, this, &MainWindow::add_mask_pixmap);

// МАСКИ
    dockMaskImage->setFloating(true);  dockMaskImage->setObjectName("dockMaskImage");
    dockMaskImage->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(dockMaskImage, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenuMaskImagList(QPoint)));
    createContextAction(QIcon(":/icons/puzzle.png"), "Загрузить 2 выделенные маски в калькулятор", 3,
                        show_mask_list_acts, SLOT(show_mask_list()));
    createContextAction(QIcon(":/icons/ok.png"), "Выделить все маски", 1,  // check all
                        show_mask_list_acts, SLOT(show_mask_list()));
    createContextAction(QIcon(":/icons/unchecked-checkbox.png"), "Снять выделение со всех масок", 2,  // uncheck all
                        show_mask_list_acts, SLOT(show_mask_list()));
    createContextAction(QIcon(":/icons/delete-32-1.png"), "Удалить все маски", 8,  // delete all
                        show_mask_list_acts, SLOT(show_mask_list()));
    action = new QAction(this);  action->setSeparator(true);  show_mask_list_acts.append(action);
    createContextAction(QIcon(":/icons/theater.png"), "Сохранить выделенные изображения маски (*.mask) ...", 4,  // save checked
                        show_mask_list_acts, SLOT(show_mask_list()));
    createContextAction(QIcon(":/icons/theater.png"), "Загрузить изображения маски (*.mask) ...", 5,  // load checked
                        show_mask_list_acts, SLOT(show_mask_list()));
    createContextAction(QIcon(":/icons/save.png"), "Сохранить выделенные маски в файлы ...", 6,  // save checked pdf png jpg jpeg csv
                        show_mask_list_acts, SLOT(show_mask_list()));
//    createContextAction(QIcon(":/icons/csv2.png"), "Сохранить выделенные изображения в Excel CSV файлы ...", 7,  // save checked csv
//                        show_mask_list_acts, SLOT(show_mask_list()));


    dockMaskImage->resize(450,scrHeignt4+18+correct_ch_mask);
    dockMaskImage->setWidget(maskListWidget);
    dockMaskImage->move(rec.width() - 250 - 200 - 30,upperIndent+3*scrHeignt4-correct_ch_mask);
    addDockWidget(Qt::BottomDockWidgetArea, dockMaskImage);

    connect(maskListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClickedMaskList(QListWidgetItem*)));
    connect(dockMaskImage->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));

// КАЛЬКУЛЯТОР МАСОЧНЫХ ИЗОБРАЖЕНИЙ
    imgMasks->hide();
    imgMasks->gsettings = &GLOBAL_SETTINGS;
    imgMasks->setLWidgetAndIHandler(maskListWidget, im_handler);
    connect(imgMasks->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
    connect(imgMasks, &imageMask::appendMask, this, &MainWindow::add_mask_pixmap);

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
    wPlot->yAxis->setRange(0, 2.6);
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
    dockw->setWindowTitle(QString("%1 [точек-%2 || ср.кв.отклонение-%3%]").arg(item->getTitle())
                          .arg(item->count_of_point).arg(item->sigma, 0, 'f', 0));

    item->setInversCm(false);
    item->dockw = dockw;
    item->imgHand = im_handler;
    item->data_file_name = dataFileName;
    dockw->setFloating(true);  dockw->setObjectName("zGraph");

    dockw->resize(GLOBAL_SETTINGS.zobject_dock_size_w,GLOBAL_SETTINGS.zobject_dock_size_h);

    wPlot = new QCustomPlot();  dockw->setWidget(wPlot);    item->plot = wPlot;

    wPlot->resize(GLOBAL_SETTINGS.zobject_dock_size_w,GLOBAL_SETTINGS.zobject_dock_size_h);

    item->setupPlotShowOptions();  // настройка отображения плавающего окна

    QCPGraph *graph_lower = wPlot->addGraph();
    graph_lower->setPen(Qt::NoPen);
    graph_lower->setSelectable(QCP::stNone);

    QCPGraph *graph_upper = wPlot->addGraph();
    graph_upper->setPen(Qt::NoPen);
    item->transparency = GLOBAL_SETTINGS.std_dev_brush_color_transparency;
    QColor bc(GLOBAL_SETTINGS.std_dev_brush_color_red,GLOBAL_SETTINGS.std_dev_brush_color_green,
              GLOBAL_SETTINGS.std_dev_brush_color_blue, item->transparency);
    graph_upper->setBrush(bc);
    graph_upper->setSelectable(QCP::stNone);
    graph_upper->setChannelFillGraph(graph_lower);

// set axes ranges, so we see all data:
    wPlot->xAxis->setRange(GLOBAL_SETTINGS.zobject_plot_xAxis_lower,GLOBAL_SETTINGS.zobject_plot_xAxis_upper);
    wPlot->yAxis->setRange(GLOBAL_SETTINGS.zobject_plot_yAxis_lower,GLOBAL_SETTINGS.zobject_plot_yAxis_upper);
    item->setContextMenuConnection();
    addDockWidget(Qt::BottomDockWidgetArea, dockw);
    dockw->hide();
    dockw->setAllowedAreas(nullptr);  // NoToolBarArea - !!! не прикрепляется к главному окну
    QListWidgetItem *lwItem = new QListWidgetItem();
    item->listwidget = lwItem;
    lwItem->setText(item->getTitle());
    QFont font = lwItem->font();  font.setBold(item->dockw->isVisible()); lwItem->setFont(font);
    lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
    lwItem->setCheckState(Qt::Checked);
    lwItem->setIcon(item->aicon);
    zGraphListWidget->insertItem(0, lwItem);

    connect(item, SIGNAL(changeParamsFromDialog(zGraph*)), this, SLOT(spectralUpdateExt(zGraph*)));
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
        zgrap_item->setVisible(true);
        zgrap_item->dockw->setVisible(!zgrap_item->dockw->isVisible());
        spectralAct->setEnabled(true);
        if (imgSpectral->isVisible()) imgSpectral->updateDataExt(dataFileName, zitems, nullptr);
    }
    else { zgrap_item->setVisible(false); zgrap_item->setSelected(false);
        zgrap_item->dockw->setVisible(false);

        calculateVisibleZObject(false);  // проверим есть ли видимые объекты

    }  // if
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
    maskAct->setChecked(imgMasks->isVisible());

}

void MainWindow::open() {
  QString fname = QFileDialog::getOpenFileName(
      this, tr("Открытие файла данных"), "../data",
      tr("ENVI HDR Файлы (*.hdr);;Файлы JPG (*.jpg *.jpeg);;Файлы PNG (*.png);;Файлы TIFF (*.tif *.tiff)"));
  if (fname.isEmpty()) return;
  if(!QFileInfo::exists(fname)) {  // Файл не найден
      QMessageBox *msgBox = new QMessageBox;
      msgBox->setWindowIcon(icon256);
      im_handler->showWarningMessageBox(msgBox,
                  QString("Файл %1 не найден !").arg(fname));
      return;
  }  // if

// jpg added
  QFileInfo info(fname);
  if ( info.suffix().toLower() == "jpg" || info.suffix().toLower() == "png" || info.suffix().toLower() == "jpeg"
    || info.suffix().toLower() == "tif" || info.suffix().toLower() == "tiff")  // выбран файл jpg jpeg png tif tiff
      fname = im_handler->getHDRfileNameConvertedFromJPG(fname);
// jpg added

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
  view->GlobalScale = 1. / GLOBAL_SETTINGS.main_rgb_scale_start;
  view->GlobalRotate = - GLOBAL_SETTINGS.main_rgb_rotate_start;
  view->scale(1/view->GlobalScale, 1/view->GlobalScale);
  view->rotate(-view->GlobalRotate);
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

    imgSpectral->hide();  spectralAct->setEnabled(false); spectralAct->setChecked(false);
    imgHistogram->hide();  histogramAct->setEnabled(false); histogramAct->setChecked(false);
    imgMasks->hide();  maskAct->setEnabled(false); maskAct->setChecked(false);

    setWindowTitle(QString("%1 - [%2]").arg(appName).arg(dataFileName));
    createDockWidgetForChannels();
    if (restoreSettingAtStartUp && index_update) restoreIndexes();  // slice++image++brightness

    createDockWidgetForIndexes();  // with RGB default + slice++image++brightness

    if (restoreSettingAtStartUp) restoreSettings();  // dock + zgraph


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
    imgSpectral->updateDataExt(dataFileName, z_list, nullptr);
}

void MainWindow::createContextAction(const QIcon &icon, const QString &text, int num, QVector<QAction *> &av, const char *member)
{
    QAction *action = new QAction(icon, text, this);
    action->setData(num);  av.append(action);
    connect(action, SIGNAL(triggered()), this, member);

}

void MainWindow::showContextMenuMaskImagList(const QPoint &pos)
{
    QPoint globalPos = maskListWidget->mapToGlobal(pos);
    QMenu menu(this);
    menu.addAction(maskAct);
    for (int i = 0; i < show_mask_list_acts.count(); i++)
        menu.addAction(show_mask_list_acts[i]);
    menu.exec(globalPos);
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
        QMessageBox *msgBox = new QMessageBox;
        msgBox->setWindowIcon(icon256);
        im_handler->showWarningMessageBox(msgBox,
                    "Для сохранения необходимо загрузить данные !");
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

void MainWindow::histogramSlot() { imgHistogram->setVisible(!imgHistogram->isVisible()); }


void MainWindow::masksSlot() {

    imgMasks->setVisible(!imgMasks->isVisible());
//    if (imgMasks->isVisible())
    imgMasksUpdatePreviewPixmap();
}


void MainWindow::spectralSlot() {
    auto z_list = view->getZGraphItemsList();  // список рафических объектов на сцене
    imgSpectral->updateDataExt(dataFileName, z_list, nullptr);
    imgSpectral->setVisible(!imgSpectral->isVisible());

}

void MainWindow::createDockWidgetForChannels()
{
    chListWidget->clear();
    uint32_t depth = im_handler->current_image()->get_bands_count();
    for(int num=0; num<depth; num++) {
        QListWidgetItem *lwItem = new QListWidgetItem();
        im_handler->current_image()->set_LW_item(lwItem, num);
        chListWidget->addItem(lwItem);
    }  // for
    chListWidget->setCurrentRow(0);
/*    chListWidget->clear();
    auto strlist = im_handler->current_image()->get_wl_list(2);  // две цифры после запятой
    auto wavelengths = im_handler->current_image()->wls();
    foreach(QString str, strlist) {
        QListWidgetItem *lwItem = new QListWidgetItem();
        chListWidget->addItem(lwItem);
        lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
        lwItem->setCheckState(Qt::Unchecked);
        lwItem->setText(str);

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
    chListWidget->setCurrentRow(0); */
}

void MainWindow::create_default_RGB_image()
{
    im_handler->current_image()->append_RGB();
    uint32_t depth = im_handler->current_image()->get_bands_count();
    view->GlobalChannelNum = depth;
    im_handler->current_image()->set_current_slice(view->GlobalChannelNum);

/*// 84 - 641nm 53 - 550nm 22 - 460nm
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
    im_handler->current_image()->set_current_slice(view->GlobalChannelNum);*/
}

void MainWindow::restoreIndexListWidget() {
    int count = im_handler->current_image()->get_image_count();
    uint32_t depth = im_handler->current_image()->get_bands_count();
    for (int i = 0; i < count; i++) {
        im_handler->current_image()->set_current_slice(depth + i);
        QListWidgetItem *lwItem = new QListWidgetItem();
        lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
        lwItem->setCheckState(Qt::Unchecked);
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
        im_handler->current_image()->set_LW_item(lwItem, view->GlobalChannelNum);
        indexListWidget->addItem(lwItem);
    }  // if

    indexListWidget->setCurrentRow(0);
    chListWidget->currentItem()->setSelected(false);  // конкурент
    set_abstract_index_pixmap();
    QSize size = im_handler->current_image()->get_raster_x_y_size();
    dockIndexList->setWindowTitle(QString("Изображения - [%1 x %2]").arg(size.width()).arg(size.height()));
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
    QSettings *settings = new QSettings( iniFileName, QSettings::IniFormat );
    QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
    settings->setIniCodec( codec );
    int ver = settings->value("version").toInt();
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
        QFile::copy(QCoreApplication::applicationDirPath () + "/configuration/" + info.completeBaseName() + ".conf", fname);
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
    GLOBAL_SETTINGS.zobjects_prof_deviation_show = settings.value( "zobjects_prof_deviation_show", true).toBool();

    GLOBAL_SETTINGS.main_rgb_rotate_start = settings.value( "main_rgb_rotate_start", 90.).toDouble();
    GLOBAL_SETTINGS.main_rgb_scale_start = settings.value( "main_rgb_scale_start", 2.).toDouble();

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
    settings.setValue( "zobjects_prof_deviation_show", GLOBAL_SETTINGS.zobjects_prof_deviation_show );
    settings.setValue( "main_rgb_rotate_start", GLOBAL_SETTINGS.main_rgb_rotate_start );
    settings.setValue( "main_rgb_scale_start", GLOBAL_SETTINGS.main_rgb_scale_start );

    settings.endGroup();

    settings.beginGroup( "Indexes" );
    foreach(J09::indexType item, indexList) settings.setValue( item.name, item.formula );
    settings.endGroup();

    // recent files
    settings.remove("Resent files");
    settings.beginGroup( "Resent files" );
    int count = 1;
    for(int i=0; i<recentFileNames.count(); i++) {
        if (count > GLOBAL_SETTINGS.resent_files_count) break;
        QString key(QString("file%1").arg(count));
        settings.setValue( key , recentFileNames[i] );
        count++;
    }
    settings.endGroup();

}

QString MainWindow::getMainSettingsFileName()
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(writableLocation);
    return writableLocation + "/" + info.completeBaseName() + ".conf";
}

void MainWindow::restoreSettingsVersionOne(QSettings *settings)
{
    view->GlobalScale = settings->value("scale", 1.0).toDouble();
    view->GlobalRotate = settings->value("rotation", 0.0).toDouble();
    view->GlobalChannelNum = settings->value("channelNum", 0).toInt();
    uint32_t depth = im_handler->current_image()->get_bands_count();
    if (view->GlobalChannelNum < depth) {
        QListWidgetItem *lwItem = chListWidget->item(view->GlobalChannelNum);
        itemClickedChannelList(lwItem);
        chListWidget->setCurrentItem(lwItem);  chListWidget->scrollToItem(lwItem);
    }  else {
        QListWidgetItem *lwItem = indexListWidget->item(view->GlobalChannelNum - depth);
        lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
        lwItem->setCheckState(Qt::Unchecked);
        itemClickedIndexList(lwItem);
        indexListWidget->setCurrentItem(lwItem);  indexListWidget->scrollToItem(lwItem);
    }  // if
    view->GlobalChannelStep = settings->value("channelStep", 1).toInt();
    show_channel_list_update();

    int horizontal = settings->value("horizontal", 0).toInt();
    int vertical = settings->value("vertical", 0).toInt();
    view->resetMatrix();  view->resetTransform();
    view->scale(1/view->GlobalScale, 1/view->GlobalScale);
    view->rotate(-view->GlobalRotate);
    view->horizontalScrollBar()->setValue(horizontal);
    view->verticalScrollBar()->setValue(vertical);
    auto graphlist = settings->value("dockZGraphList", true).toBool();
    auto channellist = settings->value("dockChannelList", true).toBool();
    auto indexlist = settings->value("dockIndexList", true).toBool();
    dockZGraphList->setVisible(graphlist);
    dockChannelList->setVisible(channellist);
    dockIndexList->setVisible(indexlist);
    view->winZGraphListAct->setChecked(dockZGraphList->isVisible());
    view->channelListAct->setChecked(dockChannelList->isVisible());
    view->indexListAct->setChecked(dockIndexList->isVisible());
    view->PAN = false;
    QStringList groups = settings->childGroups();

    createZGraphItemFromGroups(groups, settings);  // создать объекты из текстового списка

    restoreGeometry(settings->value("geometry").toByteArray());
    restoreState(settings->value("windowState").toByteArray(), 1);
    restoreTRUEdockWidgetsPosition();  // restoreGeometry !!! путает последовательность окон профилей
}

void MainWindow::createZGraphItemFromGroups(QStringList groups, QSettings *settings)
{
    foreach(QString str, groups) {
        if (str.indexOf("Point") != -1) {
            settings->beginGroup(str);
            QString pos = settings->value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zPoint *point = new zPoint( pXY );
            QString title = settings->value("title").toString();
            point->setTitle(title);
            point->aicon = QIcon(":/images/pin_grey.png");
            bool objectvisible = settings->value("objectvisible", true).toBool();
            point->setVisible(objectvisible);
// create dynamic DockWidget
            view->scene()->addItem(point);          // view->scene()->addItem(point);
            point->updateBoundingRect();
            emit view->insertZGraphItem(point);
// &&& sochi 2020
//            view->show_profile_for_Z(point);

            point->setSettingsFromFile(settings);  // ввод всех профилей без расчета !!!
//            emit view->changeZObject(point);
            point->data_file_name = dataFileName;
// &&& sochi 2020
            emit view->setZGraphDockToggled(point);
            point->dockw->setVisible(settings->value("dockvisible", true).toBool());
            QString dockwpos = settings->value("dockwpos").toString();
            point->dockwpos = getPoint__fromStr(dockwpos);
            settings->endGroup();
// create dynamic DockWidget
            continue;
        }  // Point
        if (str.indexOf("Rect") != -1) {
            settings->beginGroup(str);
            QString pos = settings->value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zRect *rect = new zRect(pXY,view->GlobalScale,view->GlobalRotate);
            QString title = settings->value("title").toString();
            rect->setTitle(title);
            rect->aicon = QIcon(":/images/vector_square.png");
            bool objectvisible = settings->value("objectvisible", true).toBool();
            rect->setVisible(objectvisible);
            double rotation = settings->value("rotation").toDouble();
            rect->setRotation(rotation);
            QString sizestr = settings->value("size").toString();
            QPointF fsize = getPointFromStr(sizestr);
            rect->frectSize.setWidth(fsize.rx());
            rect->frectSize.setHeight(fsize.ry());
// create dynamic DockWidget
            view->scene()->addItem(rect);          // view->scene()->addItem(rect);
            rect->updateBoundingRect();
            emit view->insertZGraphItem(rect);
            // &&& sochi 2020
            //            view->show_profile_for_Z(rect);
            rect->setSettingsFromFile(settings);  // ввод всех профилей без расчета !!!
//            emit view->changeZObject(rect);
            rect->data_file_name = dataFileName;
            // &&& sochi 2020
            emit view->setZGraphDockToggled(rect);
            rect->dockw->setVisible(settings->value("dockvisible", true).toBool());
            QString dockwpos = settings->value("dockwpos").toString();
            rect->dockwpos = getPoint__fromStr(dockwpos);
            settings->endGroup();
// create dynamic DockWidget
            continue;
        }  // Rect
        if (str.indexOf("Ellipse") != -1) {
            settings->beginGroup(str);
            QString pos = settings->value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zEllipse *ellipse = new zEllipse(pXY,view->GlobalScale,view->GlobalRotate);
            QString title = settings->value("title").toString();
            ellipse->setTitle(title);
            ellipse->aicon = QIcon(":/images/vector_ellipse.png");
            bool objectvisible = settings->value("objectvisible", true).toBool();
            ellipse->setVisible(objectvisible);
            double rotation = settings->value("rotation").toDouble();
            ellipse->setRotation(rotation);
            QString sizestr = settings->value("size").toString();
            QPointF fsize = getPointFromStr(sizestr);
            ellipse->frectSize.setWidth(fsize.rx());
            ellipse->frectSize.setHeight(fsize.ry());
// create dynamic DockWidget
            view->scene()->addItem(ellipse);          // view->scene()->addItem(ellipse);
            ellipse->updateBoundingRect();
            emit view->insertZGraphItem(ellipse);
            // &&& sochi 2020
            //            view->show_profile_for_Z(ellipse);
            ellipse->setSettingsFromFile(settings);  // ввод всех профилей без расчета !!!
//            emit view->changeZObject(ellipse);
            ellipse->data_file_name = dataFileName;
            // &&& sochi 2020
            emit view->setZGraphDockToggled(ellipse);
            ellipse->dockw->setVisible(settings->value("dockvisible", true).toBool());
            QString dockwpos = settings->value("dockwpos").toString();
            ellipse->dockwpos = getPoint__fromStr(dockwpos);
            settings->endGroup();
// create dynamic DockWidget
            continue;
        }  // Ellipse
        if (str.indexOf("Polyline") != -1) {
            settings->beginGroup(str);
            QString pos = settings->value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zPolyline *polyline = new zPolyline( view->GlobalScale,view->GlobalRotate );
            QString title = settings->value("title").toString();
            polyline->setTitle(title);
            polyline->setPos(pXY);
            polyline->aicon = QIcon(":/images/polyline-64.png");
            bool objectvisible = settings->value("objectvisible", true).toBool();
            polyline->setVisible(objectvisible);
            QString xstr = settings->value("x").toString();
            QString ystr = settings->value("y").toString();
            auto x = getVectorFromStr(xstr);
            auto y = getVectorFromStr(ystr);
            for (int i=0; i<x.length();i++)
                polyline->fpolygon.append(QPoint(x[i],y[i]));
// create dynamic DockWidget
            view->scene()->addItem(polyline);          // view->scene()->addItem(polyline);
            polyline->updateBoundingRect();
            emit view->insertZGraphItem(polyline);
            // &&& sochi 2020
            //            view->show_profile_for_Z(polyline);
            polyline->setSettingsFromFile(settings);  // ввод всех профилей без расчета !!!
//            emit view->changeZObject(polyline);
            polyline->data_file_name = dataFileName;
            // &&& sochi 2020
            emit view->setZGraphDockToggled(polyline);
            polyline->dockw->setVisible(settings->value("dockvisible", true).toBool());
            QString dockwpos = settings->value("dockwpos").toString();
            polyline->dockwpos = getPoint__fromStr(dockwpos);
            settings->endGroup();
// create dynamic DockWidget
            continue;
        }  // Polyline
        if (str.indexOf("Polygon") != -1) {
            settings->beginGroup(str);
            QString pos = settings->value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zPolygon *polygon = new zPolygon( view->GlobalScale,view->GlobalRotate );
            QString title = settings->value("title").toString();
            polygon->setTitle(title);
            polygon->setPos(pXY);
            polygon->aicon = QIcon(":/images/vector-polygon.png");
            bool objectvisible = settings->value("objectvisible", true).toBool();
            polygon->setVisible(objectvisible);
            QString xstr = settings->value("x").toString();
            QString ystr = settings->value("y").toString();
            auto x = getVectorFromStr(xstr);
            auto y = getVectorFromStr(ystr);
            for (int i=0; i<x.length();i++)
                polygon->fpolygon.append(QPoint(x[i],y[i]));
// create dynamic DockWidget
            view->scene()->addItem(polygon);          // view->scene()->addItem(polygon);
            polygon->updateBoundingRect();
            emit view->insertZGraphItem(polygon);
            // &&& sochi 2020
//                        view->show_profile_for_Z(polygon);
            polygon->setSettingsFromFile(settings);  // ввод всех профилей без расчета !!!
//            emit view->changeZObject(polygon);
            polygon->data_file_name = dataFileName;
            // &&& sochi 2020
            emit view->setZGraphDockToggled(polygon);
            polygon->dockw->setVisible(settings->value("dockvisible", true).toBool());
            QString dockwpos = settings->value("dockwpos").toString();
            polygon->dockwpos = getPoint__fromStr(dockwpos);
            settings->endGroup();
// create dynamic DockWidget
            continue;
        }  // Polygon
    }  // foreach
    emit view->changeZObject(nullptr);
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

void MainWindow::restoreSettingsVersionTwo(QSettings *settings)
{
    Q_UNUSED(settings)
}

void MainWindow::itemClickedChannelList(QListWidgetItem *lwItem)
{
    slider->setEnabled(true);

    indexListWidget->currentItem()->setSelected(false);  // конкурент
    QListWidgetItem *mlw = maskListWidget->currentItem();
    if(mlw) mlw->setSelected(false);  // конкурент

    view->GlobalViewMode = 0;
    int num = chListWidget->row(lwItem);
    view->GlobalChannelNum = num;
    im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
//----------------------------------------------------------------
    set_abstract_index_pixmap();
    imgHistogram->hide();  histogramAct->setEnabled(false);
}

void MainWindow::itemClickedIndexList(QListWidgetItem *lwItem)
{
    slider->setEnabled(true);

    chListWidget->currentItem()->setSelected(false);  // конкурент
    QListWidgetItem *mlw = maskListWidget->currentItem();
    if(mlw) mlw->setSelected(false);  // конкурент

    view->GlobalViewMode = 0;
    int num = indexListWidget->row(lwItem);
    uint32_t depth = im_handler->current_image()->get_bands_count();
    view->GlobalChannelNum = num + depth;
    im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
//----------------------------------------------------------------
    set_abstract_index_pixmap();
    if (num==0) {imgHistogram->hide(); histogramAct->setEnabled(false); }
    else { updateHistogram(); histogramAct->setEnabled(true); }
}

void MainWindow::itemClickedMaskList(QListWidgetItem *lwItem)
{
    slider->setEnabled(false);

    chListWidget->currentItem()->setSelected(false);  // конкурент
    indexListWidget->currentItem()->setSelected(false);  // конкурент
    view->GlobalViewMode = 1;
    int num = maskListWidget->row(lwItem);
    view->GlobalChannelNum = -1;  view->GlobalMaskNum = num;
    set_abstract_mask_pixmap();
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
    QPixmap pxmap = view->mainPixmap->pixmap();
    im_handler->current_image()->save_icon_to_file(pxmap, tb_def_size);
    fileToolBar->addAction(recentAct);
    connect(recentAct, SIGNAL(triggered()), this, SLOT(OpenRecentFile()));
    recentFileNames.append(dataFileName);
    fileMenu->insertAction(addSeparatorRecentFile, recentAct);
    view->resentFilesActions.append(recentAct);
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
    for (int i = 0; i < show_channel_list_acts.count(); i++)
        menu.addAction(show_channel_list_acts[i]);
    menu.exec(globalPos);
}

void MainWindow::showContextMenuZGraphList(const QPoint &pos)
{
    QPoint globalPos = zGraphListWidget->mapToGlobal(pos);
    QMenu menu(this);
    menu.addAction(spectralAct);
    for (int i = 0; i < show_zgraph_list_acts.count(); i++)
        menu.addAction(show_zgraph_list_acts[i]);

//    QMenu *maskMenu = menu.addMenu(tr("Отфильтровать по маске"));

//    for (int i = 0; i < mask_list_acts.count(); i++)
//        maskMenu->addAction(mask_list_acts[i]);

    menu.exec(globalPos);
}

void MainWindow::showContextMenuDockIndexList(const QPoint &pos)
{
    if (dataFileName.isEmpty()) return;
    QPoint globalPos = indexListWidget->mapToGlobal(pos);
    QMenu menu(this);
    for (int i = 0; i < show_index_list_acts.count(); i++)
        menu.addAction(show_index_list_acts[i]);

    menu.exec(globalPos);
}

void MainWindow::show_mask_list()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        int num = action->data().toInt();
        switch (num) {
        case 1 : for (int row = 0; row < maskListWidget->count(); row++)
                maskListWidget->item(row)->setCheckState(Qt::Checked);
            break;
        case 2 : for (int row = 0; row < maskListWidget->count(); row++)
                maskListWidget->item(row)->setCheckState(Qt::Unchecked);
            break;
        case 3 : { int count = imgMasks->set2MasksToForm();    // Загрузить 2 выделенные маски в калькулятор
            if (count > 0) imgMasks->show();  imgMasksUpdatePreviewPixmap();
            break; }
        case 4 : {                                              // Сохранить выделенные изображения маски (*.mask)
            auto proj_path = getDataSetPath();
            QString fname = QFileDialog::getSaveFileName(
                        this, tr("Сохранить выделенные изображения маски"), proj_path,
                        tr("Файлы изображений масок (*.mask)"));
            if (fname.isEmpty()) return;
            imgMasks->saveMasksToFile(fname, maskListWidget);
            break; }
        case 5 : {                                              // Загрузить изображения маски (*.mask)
            auto proj_path = getDataSetPath();
            if (proj_path.isEmpty()) {
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет загруженного проекта !");
                return;
            }
            QFileDialog fd;
            fd.move(dockMaskImage->pos() + QPoint(-fd.width(),dockMaskImage->height()));
            QString fname = fd.getOpenFileName(
                this, tr("Загрузка изображений масок"), proj_path,
                tr("Файлы изображений масок (*.mask)"));
            if (fname.isEmpty()) return;
            imgMasks->loadMasksFromFile(fname);
            imgMasks->updateMaskListWidget();
            maskAct->setEnabled(true);  maskAct->setChecked(true);
            imgMasks->show();  imgMasksUpdatePreviewPixmap();
            break; }
        case 6 : {            // Сохранить выделенные маски в файлы ...", 6,  // save checked pdf png jpg jpeg csv
            int count = 0;
            for(int row=0; row<maskListWidget->count(); row++)  // исключая RGB
                if(maskListWidget->item(row)->checkState() == Qt::Checked) count++;
            if (!count) {
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет выделенных масок !");
                 return;
            }  // if
            SaveFListDlg *dlg = new SaveFListDlg();
            dlg->setWindowTitle("Сохранение выделенных масок в файлы");
            dlg->ui->labelfCount->setText(QString("Выбрано %1 файлов").arg(count));
            QPoint pos = dockMaskImage->pos(); QPoint mv(-dlg->width()/2,-dlg->height());
            dlg->move(pos + mv);
            if (dlg->exec() == QDialog::Accepted) {}
            break;
        }  // 6
        case 8 : {                                              // Удалить все маски
            QMessageBox msgBox;
            updateDeleteItemsBoxWarning(msgBox, "Удаление всех масок",
              "Вы подтверждаете удаление всех масок ?\nВосстановить удаление будет невозможно !");
            QPoint pos = dockMaskImage->pos(); QPoint mv(-300,0); msgBox.move(pos + mv);

            int res = msgBox.exec();
            if (res == QMessageBox::Ok) {   // нажата кнопка Да
                imgMasks->clearForAllObjects();
                maskAct->setChecked(false); maskAct->setEnabled(false);
// режим отображения устанавливаем в RGB
                view->GlobalViewMode = 0;
                view->GlobalChannelNum = im_handler->current_image()->get_bands_count();
                im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
                set_abstract_index_pixmap();
            //----------------------------------------------------------------
                imgHistogram->hide(); histogramAct->setEnabled(false);
                chListWidget->currentItem()->setSelected(false);  // конкурент
                indexListWidget->setCurrentRow(0);
                indexListWidget->currentItem()->setSelected(true);  // RGB
            }  // if QMessageBox::Ok
            break; }
        }
    }
}

void MainWindow::show_index_list()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        int num = action->data().toInt();
        switch (num) {
        case 1 : {    // Сохранить выделенные индексные изображения  в *.index файл ...
            // проверка на наличие выделенных индексных изображений
            int count = 0;
            for(int row=1; row<indexListWidget->count(); row++)  // исключая RGB
                if(indexListWidget->item(row)->checkState() == Qt::Checked) count++;
            if (!count) {
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет выделенных индексных изображений !");
                 return;
            }  // if
            auto proj_path = getDataSetPath();
            QString fname = QFileDialog::getSaveFileName(
                this, tr("Сохранение выделенных индексных изображений"), proj_path,
                tr("Файлы индексных изображений (*.index)"));
            if (fname.isEmpty()) return;

            im_handler->current_image()->save_checked_indexes(fname);

            break;
        }  // case 1
        case 2 : {    // Загрузить индексные изображения из *.index файла ...", 2,  // load
            auto proj_path = getDataSetPath();
            if (proj_path.isEmpty()) {
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет загруженного проекта !");
                return;
            }
            QFileDialog fd;
            QString fname = fd.getOpenFileName(
                this, tr("Загрузка индексных изображений"), proj_path,
                tr("Файлы индексных изображений (*.index)"));
            if (fname.isEmpty()) return;

            int new_num = im_handler->current_image()->load_indexes_from_file(fname, indexListWidget);
            if (new_num == 0) return;

            uint32_t depth = im_handler->current_image()->get_bands_count();
            indexListWidget->setCurrentRow(new_num - depth);
            view->GlobalChannelNum = new_num;  histogramAct->setEnabled(true);

            view->GlobalViewMode = 0;
            im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
            set_abstract_index_pixmap();
            show_histogram_widget();  // histogram
            chListWidget->currentItem()->setSelected(false);  // конкурент

            break;
        }  // case 2
        case 3 : {   // Сохранить выделенные индексы в файлы ...", 3,  // save checked pdf png jpg csv
            int count = 0;
            for(int row=1; row<indexListWidget->count(); row++)  // исключая RGB
                if(indexListWidget->item(row)->checkState() == Qt::Checked) count++;
            if (!count) {
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет выделенных индексных изображений !");
                 return;
            }  // if
            SaveFListDlg *dlg = new SaveFListDlg();
            dlg->setWindowTitle("Сохранение выделенных индексов в файлы");
            dlg->ui->labelfCount->setText(QString("Выбрано %1 файлов").arg(count));
            QPoint pos = dockIndexList->pos(); QPoint mv(-dlg->width(),-dlg->height()/2);
            dlg->move(pos + mv);
            if (dlg->exec() == QDialog::Accepted) {
                if(dlg->ui->checkBoxPng->isChecked()) im_handler->current_image()->save_checked_indexes_separately_img("png");
                if(dlg->ui->checkBoxJpg->isChecked()) im_handler->current_image()->save_checked_indexes_separately_img("jpg");
                if(dlg->ui->checkBoxJpeg->isChecked()) im_handler->current_image()->save_checked_indexes_separately_img("jpeg");
                if(dlg->ui->checkBoxCsv->isChecked()) im_handler->current_image()->save_checked_indexes_separately_csv();
            }  // QDialog::Accepted
            break;
        }  // case 3
        case 5 : {   // Удалить ВСЕ индексные изображения (кроме RGB) ...", 5,  // delete ALL (!rgb)
            QMessageBox msgBox;
            updateDeleteItemsBoxWarning(msgBox, "Удаление всех индексных изображений (кроме RGB)",
              "Вы подтверждаете удаление всех индексных изображений ?\nВосстановить удаление будет невозможно !");
            QPoint pos = dockIndexList->pos(); QPoint mv(-500,0); msgBox.move(pos + mv);
            int res = msgBox.exec();
            if (res == QMessageBox::Ok) {   // нажата кнопка Да
                // режим отображения устанавливаем в RGB
                view->GlobalViewMode = 0;
                view->GlobalChannelNum = im_handler->current_image()->get_bands_count();
                im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
                set_abstract_index_pixmap();

                im_handler->current_image()->delete_all_indexes();

                QList<QListWidgetItem*> items;
                for(int row=1; row<indexListWidget->count(); row++)
                    items.append(indexListWidget->item(row));
                foreach(QListWidgetItem *item, items)
                    delete indexListWidget->takeItem(indexListWidget->row(item));
                imgHistogram->hide();
            }  // if QMessageBox::Ok
            break;
        }  // case 5
        case 6 : {   // Удалить выделенные индексные изображения (кроме RGB) ...", 6,  // delete checked (!rgb)
            QMessageBox msgBox;
            updateDeleteItemsBoxWarning(msgBox, "Удаление выделенных индексных изображений (кроме RGB)",
              "Вы подтверждаете удаление выделенных индексных изображений ?\nВосстановить удаление будет невозможно !");
            QPoint pos = dockIndexList->pos(); QPoint mv(-500,0); msgBox.move(pos + mv);
            int res = msgBox.exec();
            if (res == QMessageBox::Ok) {   // нажата кнопка Да
                // режим отображения устанавливаем в RGB
                view->GlobalViewMode = 0;
                view->GlobalChannelNum = im_handler->current_image()->get_bands_count();
                im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
                set_abstract_index_pixmap();

                im_handler->current_image()->delete_checked_indexes();

                QList<QListWidgetItem*> items;
                for(int row=1; row<indexListWidget->count(); row++)
                    if (indexListWidget->item(row)->checkState() == Qt::Checked)
                        items.append(indexListWidget->item(row));
                foreach(QListWidgetItem *item, items)
                    delete indexListWidget->takeItem(indexListWidget->row(item));
                imgHistogram->hide();
            }  // if QMessageBox::Ok
            break;
        }  // case 6
        case 7 : { for(int row=1; row<indexListWidget->count(); row++)  // Выделить все индексные изображения", 7
                indexListWidget->item(row)->setCheckState(Qt::Checked);
                break;
        }  // case 7
        case 8 : { for(int row=1; row<indexListWidget->count(); row++)  // Снять выделение со всех индексных изображений", 8
                indexListWidget->item(row)->setCheckState(Qt::Unchecked);
                break;
        }  // case 8
        }  // switch
    }  // if
}

void MainWindow::updateDeleteItemsBoxWarning(QMessageBox &mb, QString title, QString text)
{
    mb.setWindowTitle(title);
    mb.setText(text);
    mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    mb.buttons().at(0)->setText("Да");
    mb.buttons().at(1)->setText("Нет");
    mb.setIcon(QMessageBox::Critical);
    mb.setWindowIcon(icon256);
    mb.setDefaultButton(QMessageBox::Cancel);
}

void MainWindow::show_channel_list_update() {
    for (int row = 0; row < chListWidget->count(); row++) {
        if (row % view->GlobalChannelStep == 0)
            chListWidget->item(row)->setHidden(false);
        else chListWidget->item(row)->setHidden(true);
    }  // for
}

void MainWindow::show_channel_list_special_update(int t) {
    switch (t) {
    case 1 : for (int row = 0; row < chListWidget->count(); row++)
            chListWidget->item(row)->setCheckState(Qt::Checked);
        break;
    case 2 : for (int row = 0; row < chListWidget->count(); row++)
            chListWidget->item(row)->setCheckState(Qt::Unchecked);
        break;
    case 3 : for (int row = 0; row < chListWidget->count(); row++)
            if (chListWidget->item(row)->checkState() == Qt::Checked)
                chListWidget->item(row)->setHidden(false);
            else
                chListWidget->item(row)->setHidden(true);
        break;
    }

}

void MainWindow::show_channel_list()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        view->GlobalChannelStep = action->data().toInt();
        switch (view->GlobalChannelStep) {
        case 1 :
        case 2 :
        case 5 :
        case 10 : show_channel_list_update(); break;
        case 11 : show_channel_list_special_update(1); break;
        case 12 : show_channel_list_special_update(2); break;
        case 14 : show_channel_list_special_update(3); break;
        case 15 : {  // Сохранить выделенные каналы в файлы ...", 15,  // save checked pdf png jpg jpeg csv
            int count = 0;
            for(int row=0; row<chListWidget->count(); row++)  // исключая RGB
                if(chListWidget->item(row)->checkState() == Qt::Checked) count++;
            if (!count) {
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет выделенных каналов !");
                 return;
            }  // if
            SaveFListDlg *dlg = new SaveFListDlg();
            dlg->setWindowTitle("Сохранение выделенных каналов в файлы");
            dlg->ui->labelfCount->setText(QString("Выбрано %1 файлов").arg(count));
            QPoint pos = dockChannelList->pos(); QPoint mv(-dlg->width(),-dlg->height()/2);
            dlg->move(pos + mv);
            if (dlg->exec() == QDialog::Accepted) {}
            break;
        }  // 15
        case 17 : {  // Загрузить и отобразить список каналов ...", 17,  // load checked
            auto proj_path = getDataSetPath();
            if (proj_path.isEmpty()) {
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет загруженного проекта !");
                return;
            }
            QFileDialog fd;
            QString fname = fd.getOpenFileName(
                this, tr("Загрузка списка каналов"), proj_path,
                tr("Файлы спектральных каналов (*.splst)"));
            if (fname.isEmpty()) return;

            view->GlobalViewMode = 0;
            view->GlobalChannelNum = im_handler->current_image()->load_list_checked_bands(fname);
            im_handler->current_image()->set_current_slice(view->GlobalChannelNum);
            set_abstract_index_pixmap();
            chListWidget->setCurrentRow(view->GlobalChannelNum);
            indexListWidget->currentItem()->setSelected(false);  // конкурент

            break;
        }  // 17
        case 18 : {  // Сохранить список выделенных каналов ...", 18,  // save checked
            // проверка на наличие выделенных каналов
            int count = 0;
            for(int row=0; row<chListWidget->count(); row++)  // спектральных каналов
                if(chListWidget->item(row)->checkState() == Qt::Checked) count++;
            if (!count) {
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет выделенных спектральных каналов !");
                return;
            }
            auto proj_path = getDataSetPath();
            QString fname = QFileDialog::getSaveFileName(
                this, tr("Сохранение списка выделенных каналов"), proj_path,
                tr("Файлы спектральных каналов (*.splst)"));
            if (fname.isEmpty()) return;

            im_handler->current_image()->save_list_checked_bands(fname);

            break;
        }  // 18
        }  // switch
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
                spectralAct->setEnabled(true);
            }  // foreach
            return;
        }  // case 1
        case 2 : {    // Скрыть все области интереса
            foreach(zGraph *item, zGraphList) {
                item->setVisible(false);
                item->listwidget->setCheckState(Qt::Unchecked);
                imgSpectral->hide();
                spectralAct->setEnabled(false);
                spectralAct->setChecked(false);
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
                tr("CSV Файлы Excel (*.csv);;CSV Файлы Excel (*.scv)"));
            if (csv_file_name.isEmpty()) return;

            QStringList csv_list;
            QString str = "длина волны, нм;";
            int count = zGraphList[0]->w_len.count();
            foreach(zGraph *item, zGraphList) str.append(item->getTitle() + ";");
            csv_list.append(str);
            for (int i=0; i<count; i++) {
                QString str = QString(tr("%1;").arg(zGraphList[0]->keys[i]));
                foreach(zGraph *item, zGraphList)
                    str.append(QString(tr("%1;").arg(item->values[i])));
                str = str.replace('.', ',');
                csv_list.append(str);
            }  // for i
            QFile file(csv_file_name);
            file.remove();
            file.open( QIODevice::WriteOnly | QIODevice::Text );
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
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет загруженного проекта !");
                return;
            }
            QFileDialog fd;
            fd.move(dockZGraphList->pos() + QPoint(-fd.width(),dockZGraphList->height()));
            QString fname = fd.getOpenFileName(
                this, tr("Загрузка выделенных областей интереса"), proj_path,
                tr("Файлы областей интереса (*.roi);;Файлы областей интереса (*.roi)"));
            if (fname.isEmpty()) return;

            QSettings *settings = new QSettings( fname, QSettings::IniFormat );
            QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
            settings->setIniCodec( codec );
            QStringList groups = settings->childGroups();

            createZGraphItemFromGroups(groups, settings);  // создать объекты из текстового списка

            restoreDockWidgetsPositionExt();  // восстанавливает коррдинаты окон для загруженных ОИ
            auto graphList = view->getZGraphItemsList();
            if (imgSpectral->isVisible()) imgSpectral->updateDataExt(dataFileName, graphList, nullptr);  // обновление всех профилей
            if (graphList.count() > 0) spectralAct->setEnabled(true);

            return;
        }  // case 6
        case 7 : {    // Сохранить выделенные области интереса в файл ...
            auto proj_path = getDataSetPath();
            if (proj_path.isEmpty()) {
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет загруженного проекта !");
                 return;
            }
// проверка на наличие сохраняемых областей интереса
            auto graphList = view->getZGraphItemsList();
            int count = 0;
            foreach(zGraph *item, graphList) if (item->isVisible()) count++;
            if (!count) {
                QMessageBox *msgBox = new QMessageBox;
                msgBox->setWindowIcon(icon256);
                im_handler->showWarningMessageBox(msgBox,
                            " Нет выделенных областей интереса !");
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
            updateDeleteItemsBoxWarning(msgBox, "Удаление всех областей интереса",
              "Вы подтверждаете удаление всех областей интереса ?\nВосстановить удаление будет невозможно !");
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
        calculateIndexAndStoreToIndexList(title, formula);
    }  // if
}

void MainWindow::settingsDlgShow()
{
    settingsDialog *dlg = new settingsDialog(this);
    dlg->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
    dlg->ui->buttonBox->buttons().at(1)->setText("Отмена");
    dlg->setGlobalSettings(GLOBAL_SETTINGS);
    if (dlg->exec() == QDialog::Accepted) {
        dlg->getGlobalSettings(GLOBAL_SETTINGS);
    }  // if
}

void MainWindow::change_brightness(int value)
{
    QPixmap pxm;
    QImage img = im_handler->current_image()->get_current_image();
    double brightness = im_handler->get_new_brightness(slider, value);
    im_handler->current_image()->set_current_brightness(brightness);
    pxm = view->changeBrightnessPixmap(img, brightness);
    view->mainPixmap->setPixmap(pxm);

/*    QPixmap pxm;
    QImage img = im_handler->get_REAL_current_image();
    double brightness = im_handler->get_new_brightness(slider, value);
    pxm = view->changeBrightnessPixmap(img, brightness);
    view->mainPixmap->setPixmap(pxm); */
}

void MainWindow::add_index_pixmap(int num)
{
    view->GlobalViewMode = 0;

    view->GlobalChannelNum = num;  histogramAct->setEnabled(true);

//    QImage img = im_handler->current_image()->get_index_rgb(true,true,num);
//    im_handler->current_image()->append_additional_image(img,
//                                 view->index_title_str,view->index_formula_str);
    im_handler->current_image()->set_current_slice(view->GlobalChannelNum);

//    im_handler->current_image()->histogram[view->GlobalChannelNum-1]
//            .rotation = GLOBAL_SETTINGS.main_rgb_rotate_start;
    im_handler->current_image()->set_formula_for_index(GLOBAL_SETTINGS.main_rgb_rotate_start,
                                                       view->index_title_str,view->index_formula_str);
    QListWidgetItem *lwItem = new QListWidgetItem();
    indexListWidget->addItem(lwItem);
    im_handler->current_image()->set_LW_item(lwItem, view->GlobalChannelNum);
    indexListWidget->setCurrentRow(indexListWidget->count() - 1);
    chListWidget->currentItem()->setSelected(false);  // конкурент

/*    QListWidgetItem *lwItem = new QListWidgetItem();
    lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
    lwItem->setCheckState(Qt::Unchecked);
    indexListWidget->addItem(lwItem);
    lwItem->setIcon(QIcon(":/icons/palette.png"));
    QPair<QString, QString> formula = im_handler->current_image()->get_current_formula();
    lwItem->setText(formula.first);  lwItem->setToolTip(formula.second);
    indexListWidget->setCurrentRow(indexListWidget->count() - 1);
    chListWidget->currentItem()->setSelected(false);  // конкурент*/
//    (r640-r680)/(640+r680)
//----------------------------------------------------------
    set_abstract_index_pixmap();
    show_histogram_widget();  // histogram
}

void MainWindow::add_mask_pixmap(slice_magic *sm)
{
    view->GlobalViewMode = 1;
    im_handler->current_image()->append_mask(sm);
    QListWidgetItem *item = imgMasks->createMaskWidgetItem(sm, sm->get_image());
    maskListWidget->addItem(item);
    QPixmap pxm = QPixmap::fromImage(sm->get_image());
    view->mainPixmap->setPixmap(pxm);
}

void MainWindow::set_abstract_index_pixmap()
{
    double brightness = im_handler->current_image()->get_current_brightness();
    im_handler->set_brightness_to_slider(slider, brightness);
    QImage img = im_handler->current_image()->get_current_image();
    QPixmap pxm = view->changeBrightnessPixmap(img, brightness);
    view->mainPixmap->setPixmap(pxm);

/*    double brightness = im_handler->current_image()->get_current_brightness();
    im_handler->set_brightness_to_slider(slider, brightness);
    QImage img = im_handler->get_REAL_current_image();
    QPixmap pxm = view->changeBrightnessPixmap(img, brightness);
    view->mainPixmap->setPixmap(pxm); */
}

void MainWindow::set_abstract_mask_pixmap()
{
    im_handler->current_image()->set_current_mask_index(view->GlobalMaskNum);
//    J09::maskRecordType *am = im_handler->current_image()->getCurrentMask();
    QImage img = im_handler->current_image()->get_current_mask_image();
    QPixmap pxm = QPixmap::fromImage(img);
    view->mainPixmap->setPixmap(pxm);
}

void MainWindow::show_histogram_widget() {

    updateHistogram();
}

void MainWindow::updateHistogram()
{
    double brightness = im_handler->current_image()->get_index_brightness(0);  // RGB - slice
    QImage img = im_handler->current_image()->get_additional_image(0);   // RGB - image
    mainPixmap = view->changeBrightnessPixmap(img, brightness);
    imgHistogram->setPreviewPixmap(mainPixmap);
    imgHistogram->updateData(dataFileName, im_handler->current_image()->get_current_index());
    if (histogramAct->isChecked()) imgHistogram->show();
    else imgHistogram->hide();


/*    uint32_t depth = im_handler->current_image()->get_bands_count();
    double brightness = im_handler->current_image()->getBrightness(depth);  // RGB - slice
    QImage img = im_handler->current_image()->get_additional_image(0);   // RGB - image
    mainPixmap = view->changeBrightnessPixmap(img, brightness);

    QVector<QVector<double> > slice_index = im_handler->current_image()->get_band(view->GlobalChannelNum-1);
    QPair<QString, QString> name = im_handler->current_image()->get_current_formula();

    imgHistogram->setPreviewPixmap(mainPixmap);
    imgHistogram->updateData(dataFileName, name.first, name.second, slice_index,
                             im_handler->current_image()->histogram[view->GlobalChannelNum-1]);
    if (histogramAct->isChecked()) imgHistogram->show();
    else imgHistogram->hide(); */
}

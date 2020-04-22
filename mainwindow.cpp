#include "mainwindow.h"
#include "zgraphparamdlg.h"
#include "ui_zgraphparamdlg.h"

#include <QMenu>
#include <QToolBar>
#include <QtWidgets>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  SetupUi();
  setWindowTitle(QString("%1").arg(appName));
  QApplication::setApplicationName(appName);
//    QCoreApplication.setApplicationName(ORGANIZATION_NAME)
//    QCoreApplication.setOrganizationDomain(ORGANIZATION_DOMAIN)
  if (restoreSettingAtStartUp) restoreSettings(dataFileName);
}

MainWindow::~MainWindow() {
  delete scene;
  delete view;
  worker_thread->wait(200);
  delete im_handler;
  delete worker_thread;
  //    delete progress_dialog;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event);
  if (saveSettingAtCloseApp) saveSettings(dataFileName);
}

void MainWindow::saveSettings(QString fname)
{
    if (fname.isEmpty()) return;
    QFileInfo info(fname);
    QString iniFileName = info.path() + '/' + info.completeBaseName() + ".ini";
    QSettings settings( iniFileName, QSettings::IniFormat );
    QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
    settings.setIniCodec( codec );
    settings.clear();
    settings.setValue("version", 1);
    settings.setValue("scale", view->GlobalScale);
    settings.setValue("rotation", view->GlobalRotate);
    settings.setValue("channel", view->GlobalChannelNum);
    settings.setValue("horizontal", view->horizontalScrollBar()->value());
    settings.setValue("vertical", view->verticalScrollBar()->value());
    settings.setValue("dockZGraphList", dockZGraphList->isVisible());
    settings.setValue("dockChannelList", dockChannelList->isVisible());
    auto graphList = view->getZGraphItemsList();
    int num = 0;
    foreach(zGraph *zg, graphList) {
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
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState(1));

}

void MainWindow::SetupUi() {
  setWindowTitle("gelion");
  QRect desktop = QApplication::desktop()->screenGeometry();
//  qDebug() << desktop;
  int sw = desktop.width();  int sh = desktop.height();
  int left = round(sw*(1-qmainWindowScale)*0.5+200);
  int top = round(sh*(1-qmainWindowScale)*0.5);
  int width = round(sw*qmainWindowScale*.75);
  int height = round(sh*qmainWindowScale);
//  qDebug() << qmainWindowScale << sw << sh << left << top << width << height;
  move( left, top );
  resize( width, height );
  const QIcon icon =
      QIcon::fromTheme("mainwindow", QIcon(":/icons/256_colors.png"));
  setWindowIcon(icon);
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

  im_handler->moveToThread(worker_thread);
  connect(this, SIGNAL(read_file(QString)), im_handler,
          SLOT(read_envi_hdr(QString)), Qt::DirectConnection);
  connect(im_handler, SIGNAL(numbands_obtained(int)), this,
          SLOT(show_progress(int)), Qt::DirectConnection);
  connect(im_handler, SIGNAL(finished()), this, SLOT(add_envi_hdr_pixmap()));
  connect(view, SIGNAL(point_picked(QPointF)), this,
          SLOT(show_profile(QPointF)));
  connect(view, &gQGraphicsView::roi_position_updated, this,
          [this](QPair<int, QPointF> new_pos) {
            show_profile(new_pos.second, new_pos.first);
          });
  connect(view,SIGNAL(insertZGraphItem(zGraph*)),this,SLOT(createDockWidgetForItem(zGraph*)));
  connect(view,SIGNAL(setZGraphDockToggled(zGraph*)),this,SLOT(setZGraphDockToggled(zGraph*)));

  connect(scene,SIGNAL(selectionChanged()),view,SLOT(selectionZChanged()));
  connect(view->winZGraphListAct, SIGNAL(triggered()), this, SLOT(winZGraphList()));
  connect(view->channelListAct, SIGNAL(triggered()), this, SLOT(channelList()));
  connect(view->winZGraphListShowAllAct, SIGNAL(triggered()), this, SLOT(winZGraphProfilesShowAll()));
  connect(view->winZGraphListHideAllAct, SIGNAL(triggered()), this, SLOT(winZGraphProfilesHideAll()));
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
    wPlot->yAxis->setRange(0, 1.2);
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
    QDockWidget *dockw = new QDockWidget(item->getTitle(), this);
    item->dockw = dockw;
    dockw->setFloating(true);  dockw->setObjectName("zGraph ");
    dockw->resize(420,150);
    QRect desktop = QApplication::desktop()->screenGeometry();
    dockw->move(round(desktop.width()/5),round(desktop.height()/5));
    wPlot = new QCustomPlot();  dockw->setWidget(wPlot);
    wPlot->resize(420,150);  wPlot->addGraph();
// give the axes some labels:
    wPlot->xAxis->setLabel("длина волны, нм");
    wPlot->yAxis->setLabel("коэффициент отражения");
// set axes ranges, so we see all data:
    wPlot->xAxis->setRange(390, 1010);
    wPlot->yAxis->setRange(0, 1.2);
    item->plot = wPlot;
    addDockWidget(Qt::BottomDockWidgetArea, dockw);
    dockw->hide();
    QListWidgetItem *lwItem = new QListWidgetItem();
    lwItem->setText(item->getTitle());
    QFont font = lwItem->font();  font.setBold(true);
    lwItem->setFont(font);
    lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
    lwItem->setCheckState(Qt::Checked);
    lwItem->setIcon(item->aicon);
    zGraphListWidget->insertItem(0, lwItem);
}

void MainWindow::setZGraphDockToggled(zGraph *item)
{
    connect(item->dockw->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
}

void MainWindow::winZGraphList()
{
    dockZGraphList->setVisible(!dockZGraphList->isVisible());
}

void MainWindow::channelList()
{
    dockChannelList->setVisible(!dockChannelList->isVisible());
}

void MainWindow::winZGraphProfilesShowAll()
{
//    QList<QGraphicsItem *> items = view->scene()->items();
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
    if (!view->zcRects.isEmpty()) return;
    if (!view->tmpLines.isEmpty()) return;
    int num = zGraphListWidget->row(item);
//    QList<QGraphicsItem *> items = view->scene()->items();
    QList<zGraph *> items = view->getZGraphItemsList();
    if (item->checkState() == Qt::Checked) items[num]->setVisible(true);
    else items[num]->setVisible(false);
}

void MainWindow::zGparhEditDialog(QListWidgetItem *item, zGraph *it)
{
    zgraphParamDlg *dlg = new zgraphParamDlg(this);
    dlg->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
    dlg->ui->lineEditTitle->setText(it->getTitle());
    dlg->ui->checkBoxWdock->setChecked(it->dockw->isVisible());
    if (dlg->exec() == QDialog::Accepted) {
        QString str = dlg->ui->lineEditTitle->text();
        it->setTitle(str);  it->dockw->setWindowTitle(str);
        it->updateBoundingRect();
        it->dockw->setVisible(dlg->ui->checkBoxWdock->isChecked());
        item->setText(str);
        QFont font = item->font();  font.setBold(dlg->ui->checkBoxWdock->isChecked());
        item->setFont(font);
    }  // if
}

void MainWindow::listWidgetDoubleClicked(QListWidgetItem *item)
{
    int num = zGraphListWidget->row(item);
    auto zGraphList = view->getZGraphItemsList();
    zGraph *it = zGraphList[num];
    zGparhEditDialog(item, it);
}

void MainWindow::createActions() {
// file
  fileMenu = menuBar()->addMenu(tr("&Файл"));
  fileToolBar = addToolBar(tr("Файл"));
  fileToolBar->setObjectName("fileToolBar");

  fileMenu->addAction(view->openAct);
  connect(view->openAct, &QAction::triggered, this, &MainWindow::open);
  fileMenu->addSeparator();
  addSeparatorRecentFile = fileMenu->addSeparator();
  fileMenu->addAction(view->closeAct);
  connect(view->closeAct, &QAction::triggered, this, &MainWindow::close);
  fileToolBar->addAction(view->openAct);
  fileToolBar->addSeparator();
// edit
  QMenu *editMenu = menuBar()->addMenu(tr("&Редактирование"));
  QToolBar *editToolBar = addToolBar(tr("Редактирование"));
  editToolBar->setObjectName("editToolBar");

  const QIcon printIcon =
      QIcon::fromTheme("Печать", QIcon(":/icons/butterfly.png"));
  QAction *printAct = new QAction(printIcon, tr("&Печать"), this);
  editMenu->addAction(printAct);
  editToolBar->addAction(printAct);
  auto testAction = new QAction("test");
  editToolBar->addAction(testAction);
  connect(testAction, &QAction::triggered, [&](){
    im_handler->get_index_raster("R(500) / R(700)");
    QImage img = im_handler->current_image()->get_grayscale(true);
    img.save("test.png");
  });
// &&& inset items menu
  QMenu *itemsMenu = menuBar()->addMenu("Область интереса (ОИ)");
  QToolBar *itemsToolBar = addToolBar("Область интереса (ОИ)");
  itemsToolBar->setObjectName("itemsToolBar");

  itemsMenu->addAction(view->pointAct);   itemsToolBar->addAction(view->pointAct);
  itemsMenu->addAction(view->rectAct);    itemsToolBar->addAction(view->rectAct);
  itemsMenu->addAction(view->ellipseAct); itemsToolBar->addAction(view->ellipseAct);
  itemsMenu->addAction(view->polylineAct);    itemsToolBar->addAction(view->polylineAct);
  itemsMenu->addAction(view->polygonAct); itemsToolBar->addAction(view->polygonAct);
// &&& windows
  QMenu *winMenu = menuBar()->addMenu("Окна");
  QToolBar *winToolBar = addToolBar(winMenu->title());
  winToolBar->setObjectName("winToolBar");

  winMenu->addAction(view->winZGraphListAct);   winToolBar->addAction(view->winZGraphListAct);
  winMenu->addAction(view->channelListAct);   winToolBar->addAction(view->channelListAct);
  winMenu->addAction(view->winZGraphListShowAllAct);   winToolBar->addAction(view->winZGraphListShowAllAct);
  winMenu->addAction(view->winZGraphListHideAllAct);   winToolBar->addAction(view->winZGraphListHideAllAct);
  winMenu->addSeparator();      winToolBar->addSeparator();
}

void MainWindow::createStatusBar() {
    statusBar()->showMessage("Ctrl+Скролл-масштабирование*Скролл-перемотка верт.*Alt+Скролл-перемотка гориз.*Левая кн.мышь-перемещение");
}

void MainWindow::toggleViewAction(bool b)
{
    Q_UNUSED(b);
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
    view->channelListAct->setChecked(dockChannelList->isVisible());
}

/*void QMainWindow::splitDockWidget ( QDockWidget * first, QDockWidget * second, Qt::Orientation orientation )*/

void MainWindow::createConstDockWidgets()
{
    dockZGraphList->setFloating(true);  dockZGraphList->setObjectName("dockZGraphList");
    dockZGraphList->setFixedSize(220,200);
    dockZGraphList->setWidget(zGraphListWidget);
    dockZGraphList->move(1500,770);
    addDockWidget(Qt::BottomDockWidgetArea, dockZGraphList);
    connect(dockZGraphList->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
    connect(zGraphListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listWidgetClicked(QListWidgetItem*)));
    connect(zGraphListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(listWidgetDoubleClicked(QListWidgetItem*)));
    dockChannelList->setFloating(true);  dockChannelList->setObjectName("dockChannelList");
    dockChannelList->setFixedSize(220,700);
    dockChannelList->setWidget(chListWidget);
    dockChannelList->move(1500,30);
    addDockWidget(Qt::BottomDockWidgetArea, dockChannelList);
    connect(dockChannelList->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
}

void MainWindow::open() {
  QString fname = QFileDialog::getOpenFileName(
      this, tr("Открытие файла данных"), "../data",
      tr("ENVI HDR Files (*.hdr);;ENVI HDR Files (*.hdr)"));
  dataFileName = fname;
  QStringList fnames = im_handler->get_image_file_names();
  int num = fnames.indexOf(dataFileName);
  if (num != -1) {
    if (im_handler->set_current_image(num)) {
      updateNewDataSet();
      addRecentFile();
      return;
    }
  }
  emit read_file(dataFileName);
}

void MainWindow::show_progress(int max_progress) {
  //    QProgressDialog *progress_dialog = new QProgressDialog("Загрузка файла
  //    ...", "Отмена", 0, max_progress, this);
  if (progress_dialog != nullptr) delete progress_dialog;
  progress_dialog =
      new QProgressDialog("Загрузка файла ...", "Отмена", 0, 100, this);
  progress_dialog->resize(350, 100);
  progress_dialog->setMaximum(max_progress);
  progress_dialog->show();
  QApplication::processEvents();
  connect(im_handler, SIGNAL(reading_progress(int)), progress_dialog,
          SLOT(setValue(int)));
  connect(im_handler, SIGNAL(finished()), this, SLOT(delete_progress_dialog()),
          Qt::DirectConnection);
  connect(progress_dialog, SIGNAL(canceled()), this, SLOT(stop_reading_file()),
          Qt::DirectConnection);
}

void MainWindow::stop_reading_file() {
  im_handler->set_read_file_canceled();
  delete_progress_dialog();
//  progress_dialog->hide();
  QApplication::processEvents();
}

void MainWindow::delete_progress_dialog() {
  if (progress_dialog != nullptr) {
    delete progress_dialog;
    progress_dialog = nullptr;
  }
}

void MainWindow::createDockWidgetForChannels()
{
    chListWidget->clear();
    auto strlist = im_handler->current_image()->get_wl_list(2);
    auto wavelengths = im_handler->current_image()->wls();
    foreach(QString str, strlist) {
        QListWidgetItem *lwItem = new QListWidgetItem();
        lwItem->setText(str);
        chListWidget->addItem(lwItem);
        int lwnum = strlist.indexOf(str) - 1;
        if (lwnum >= 0) {
            qreal wlen = wavelengths[lwnum];
            if (wlen > 650) continue;
            int wlenfname = round(wlen);
            lwItem->setIcon(QIcon(QString("../BPLA/images/rainbow/%1.png").arg(wlenfname)));
        } else
            lwItem->setIcon(QIcon(QString("../BPLA/images/rainbow/%1.png").arg("RGB")));
    }  // for
    chListWidget->setCurrentRow(0);
    connect(chListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClickedChannelList(QListWidgetItem*)));
}

QPointF MainWindow::getPointFromStr(QString str) {
    QStringList pos = str.split(",");
    return QPointF(pos[0].toDouble(),pos[1].toDouble());
}

QVector<double> MainWindow::getVectorFromStr(QString str) {
    QStringList strlist = str.split(",");
    QVector<double> v;
    foreach(QString s, strlist) v.append(s.toDouble());
    return v;
}

void MainWindow::restoreSettings(QString fname)
{
    if (fname.isEmpty()) return;
    QFileInfo info(fname);
    QString iniFileName = info.path() + '/' + info.completeBaseName() + ".ini";
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

void MainWindow::restoreSettingsVersionOne(QSettings &settings)
{
    view->GlobalScale = settings.value("scale", 1.0).toDouble();
    view->GlobalRotate = settings.value("rotation", 0.0).toDouble();
    view->GlobalChannelNum = settings.value("channel", 0).toInt();
    QListWidgetItem *lwItem = chListWidget->item(view->GlobalChannelNum);
    itemClickedChannelList(lwItem);
    chListWidget->setCurrentItem(lwItem);  chListWidget->scrollToItem(lwItem);
    int horizontal = settings.value("horizontal", 0).toInt();
    int vertical = settings.value("vertical", 0).toInt();
    view->scale(1/view->GlobalScale, 1/view->GlobalScale);
    view->rotate(-view->GlobalRotate);
    view->horizontalScrollBar()->setValue(horizontal);
    view->verticalScrollBar()->setValue(vertical);
    auto graphlist = settings.value("dockZGraphList", true).toBool();
    auto channellist = settings.value("dockChannelList", true).toBool();
    dockZGraphList->setVisible(graphlist);  dockChannelList->setVisible(channellist);
    view->winZGraphListAct->setChecked(dockZGraphList->isVisible());
    view->channelListAct->setChecked(dockChannelList->isVisible());
    view->PAN = false;
    QStringList groups = settings.childGroups();
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
            view->scene()->addItem(point);
            point->updateBoundingRect();
            settings.endGroup();
            emit view->insertZGraphItem(point);
            view->show_profile_for_Z(point);
            emit view->setZGraphDockToggled(point);
            point->dockw->setVisible(settings.value("dockvisible", true).toBool());
// create dynamic DockWidget
            continue;
        }  // Point
        if (str.indexOf("Rect") != -1) {
            settings.beginGroup(str);
            QString pos = settings.value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zRect *rect = new zRect(pXY,view->GlobalScale,view->GlobalRotate);
            QString title = settings.value("title").toString();
            rect->setTitle(title);
            rect->aicon = QIcon(":/images/vector_square.png");
            bool objectvisible = settings.value("objectvisible", true).toBool();
            rect->setVisible(objectvisible);
            int rotation = settings.value("rotation").toInt();
            rect->setRotation(rotation);
            QString sizestr = settings.value("size").toString();
            QPointF fsize = getPointFromStr(sizestr);
            rect->frectSize.setWidth(fsize.rx());
            rect->frectSize.setHeight(fsize.ry());
// create dynamic DockWidget
            view->scene()->addItem(rect);
            rect->updateBoundingRect();
            settings.endGroup();
            emit view->insertZGraphItem(rect);
            view->show_profile_for_Z(rect);
            emit view->setZGraphDockToggled(rect);
            rect->dockw->setVisible(settings.value("dockvisible", true).toBool());
// create dynamic DockWidget
            continue;
        }  // Rect
        if (str.indexOf("Ellipse") != -1) {
            settings.beginGroup(str);
            QString pos = settings.value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zEllipse *ellipse = new zEllipse(pXY,view->GlobalScale,view->GlobalRotate);
            QString title = settings.value("title").toString();
            ellipse->setTitle(title);
            ellipse->aicon = QIcon(":/images/vector_ellipse.png");
            bool objectvisible = settings.value("objectvisible", true).toBool();
            ellipse->setVisible(objectvisible);
            int rotation = settings.value("rotation").toInt();
            ellipse->setRotation(rotation);
            QString sizestr = settings.value("size").toString();
            QPointF fsize = getPointFromStr(sizestr);
            ellipse->frectSize.setWidth(fsize.rx());
            ellipse->frectSize.setHeight(fsize.ry());
// create dynamic DockWidget
            view->scene()->addItem(ellipse);
            ellipse->updateBoundingRect();
            settings.endGroup();
            emit view->insertZGraphItem(ellipse);
            view->show_profile_for_Z(ellipse);
            emit view->setZGraphDockToggled(ellipse);
            ellipse->dockw->setVisible(settings.value("dockvisible", true).toBool());
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
            view->scene()->addItem(polyline);
            polyline->updateBoundingRect();
            settings.endGroup();
            emit view->insertZGraphItem(polyline);
            view->show_profile_for_Z(polyline);
            emit view->setZGraphDockToggled(polyline);
            polyline->dockw->setVisible(settings.value("dockvisible", true).toBool());
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
            view->scene()->addItem(polygon);
            polygon->updateBoundingRect();
            settings.endGroup();
            emit view->insertZGraphItem(polygon);
            view->show_profile_for_Z(polygon);
            emit view->setZGraphDockToggled(polygon);
            polygon->dockw->setVisible(settings.value("dockvisible", true).toBool());
// create dynamic DockWidget
            continue;
        }  // Polygon
    }  // foreach
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray(), 1);
}

void MainWindow::restoreSettingsVersionTwo(QSettings &settings)
{
    Q_UNUSED(settings);
}

void MainWindow::itemClickedChannelList(QListWidgetItem *lwItem)
{
    int num = chListWidget->row(lwItem);
    view->GlobalChannelNum = num;
    QImage img;
    if (num == 0) img = im_handler->current_image()->get_rgb(true,84,53,22);
    else img = im_handler->current_image()->get_rgb(true,num - 1,num - 1,num - 1);
//    QPixmap pxm = QPixmap::fromImage(img);
    QPixmap pxm;
    if (num < 84) pxm = view->changeBrightnessPixmap(img, 4.0);
    else pxm = view->changeBrightnessPixmap(img, 3.0);
    view->mainPixmap->setPixmap(pxm); // p->type(); enum { Type = 7 };
}

void MainWindow::add_envi_hdr_pixmap() {
  updateNewDataSet();
  addRecentFile();
}

void MainWindow::updateNewDataSet() {
  view->clearForAllObjects();
  zGraphListWidget->clear();
  QImage img = im_handler->current_image()->get_rgb(true, 84, 53, 22);
  view->GlobalChannelNum = 0;
  QPixmap pxm = view->changeBrightnessPixmap(img, 4.0);
  view->mainPixmap->setPixmap(pxm);
  view->mainPixmap->setOpacity(1.0);
  createDockWidgetForChannels();
  setWindowTitle(QString("%1 - [%2]").arg(appName).arg(dataFileName));
  if (restoreSettingAtStartUp) restoreSettings(dataFileName);
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
        } else {
            num = im_handler->set_current_image(num);
            if (num == -1) return;
            dataFileName = im_handler->current_image()->get_file_name();
            updateNewDataSet();
        }  // if (num == -1)
    }  // if (action)
}

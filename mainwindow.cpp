#include "mainwindow.h"
#include "zgraphparamdlg.h"
#include "ui_zgraphparamdlg.h"

#include <QMenu>
#include <QToolBar>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    SetupUi();
}

MainWindow::~MainWindow() {
  delete scene;
  delete view;
  worker_thread->wait(2000);
  delete im_handler;
  delete worker_thread;
  //    delete progress_dialog;
}

void MainWindow::SetupUi() {
  setWindowTitle("gelion");
  const QIcon icon =
      QIcon::fromTheme("mainwindow", QIcon(":/icons/256_colors.png"));
  setWindowIcon(icon);
  resize(900, 700);
  scene = new QGraphicsScene(this);
  view = new gQGraphicsView(scene);
  view->imgHand = im_handler;
  setCentralWidget(view);
  QPixmap test(QString(":/images/view.jpg"));
  QGraphicsPixmapItem *p = scene->addPixmap(test);  // p->type(); enum { Type = 7 };

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
  QVector<QPointF> v = im_handler->get_profile(QPoint(px, py));
  if (v.length() == 0) return;

  int d = im_handler->get_bands_count();

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
    dockw->setFloating(true);
    dockw->resize(420,150);
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
    listWidget->insertItem(0, lwItem);
    connect(item, SIGNAL(mouseDblClick(zGraph*)), this, SLOT(zGraphMouseDblClick(zGraph*)));
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
    QList<QGraphicsItem *> items = view->scene()->items();
    foreach (QGraphicsItem *it, items) {
        if (it->type() == QGraphicsPixmapItem::Type) continue;
        if (it->type() == zContourRect::Type) continue;
        zGraph *zitem = qgraphicsitem_cast<zGraph *>(it);
        zitem->dockw->setVisible(true);
    }  // for
}

void MainWindow::winZGraphProfilesHideAll()
{
    QList<QGraphicsItem *> items = view->scene()->items();
    foreach (QGraphicsItem *it, items) {
        if (it->type() == QGraphicsPixmapItem::Type) continue;
        if (it->type() == zContourRect::Type) continue;
        zGraph *zitem = qgraphicsitem_cast<zGraph *>(it);
        zitem->dockw->setVisible(false);
    }  // for
}

void MainWindow::listWidgetClicked(QListWidgetItem *item)
{
    if (!view->zcRects.isEmpty()) return;
    int num = listWidget->row(item);
    QList<QGraphicsItem *> items = view->scene()->items();
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

void MainWindow::zGraphMouseDblClick(zGraph *it)
{
//    if (!view->zcRects.isEmpty()) return;
//    if (!view->tmpLines.isEmpty()) return;
//    QList<QGraphicsItem *> items = view->scene()->items();
    auto zGraphList = view->getZGraphItemsList();
    int num = zGraphList.indexOf(it);
    QListWidgetItem *item = listWidget->item(num);
    zGparhEditDialog(item, it);
}

void MainWindow::listWidgetDoubleClicked(QListWidgetItem *item)
{
//    if (!view->zcRects.isEmpty()) return;
//    if (!view->tmpLines.isEmpty()) return;
    int num = listWidget->row(item);
//    QList<QGraphicsItem *> items = view->scene()->items();
//    zGraph *it = qgraphicsitem_cast<zGraph *>(items[num]);
    auto zGraphList = view->getZGraphItemsList();
    zGraph *it = zGraphList[num];
    zGparhEditDialog(item, it);
}

void MainWindow::createActions() {
  // file
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  QToolBar *fileToolBar = addToolBar(tr("Файл"));

  fileMenu->addAction(view->openAct);
  connect(view->openAct, &QAction::triggered, this, &MainWindow::open);
  fileMenu->addSeparator();
  fileMenu->addAction(view->closeAct);
  connect(view->closeAct, &QAction::triggered, this, &MainWindow::close);
  fileToolBar->addAction(view->openAct);
  // edit
  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
  QToolBar *editToolBar = addToolBar(tr("Редактирование"));
  const QIcon printIcon =
      QIcon::fromTheme("Печать", QIcon(":/icons/butterfly.png"));
  QAction *printAct = new QAction(printIcon, tr("&Печать"), this);
  editMenu->addAction(printAct);
  editToolBar->addAction(printAct);
  // &&& inset items menu
  QMenu *itemsMenu = menuBar()->addMenu("Область интереса (ОИ)");
  QToolBar *itemsToolBar = addToolBar("Область интереса (ОИ)");
  itemsMenu->addAction(view->pointAct);   itemsToolBar->addAction(view->pointAct);
  itemsMenu->addAction(view->rectAct);    itemsToolBar->addAction(view->rectAct);
  itemsMenu->addAction(view->ellipseAct); itemsToolBar->addAction(view->ellipseAct);
  itemsMenu->addAction(view->polylineAct);    itemsToolBar->addAction(view->polylineAct);
  itemsMenu->addAction(view->polygonAct); itemsToolBar->addAction(view->polygonAct);
  // &&& windows
  QMenu *winMenu = menuBar()->addMenu("Окна");
  QToolBar *winToolBar = addToolBar(winMenu->title());
  winMenu->addAction(view->winZGraphListAct);   winToolBar->addAction(view->winZGraphListAct);
  winMenu->addAction(view->channelListAct);   winToolBar->addAction(view->channelListAct);
  winMenu->addAction(view->winZGraphListShowAllAct);   winToolBar->addAction(view->winZGraphListShowAllAct);
  winMenu->addAction(view->winZGraphListHideAllAct);   winToolBar->addAction(view->winZGraphListHideAllAct);
  winMenu->addSeparator();      winToolBar->addSeparator();

}

/*QDockWidget *dock = new QDockWidget(tr("Customers"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    customerList = new QListWidget(dock);
    customerList->addItems(QStringList()
            << "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton"
    dock->setWidget(customerList);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
*/

void MainWindow::createStatusBar() {
    statusBar()->showMessage("Ctrl+Скролл-масштабирование*Скролл-перемотка верт.*Alt+Скролл-перемотка гориз.*Левая кн.мышь-перемещение");
}

void MainWindow::toggleViewAction(bool b)
{
    Q_UNUSED(b);
    QList<QGraphicsItem *> items = view->scene()->items();
    int num = 0;
    foreach (QGraphicsItem *it, items) {
        if (it->type() == QGraphicsPixmapItem::Type) continue;
        if (it->type() == zContourRect::Type) continue;
        if (it->type() == QGraphicsLineItem::Type) continue;
        zGraph *zitem = qgraphicsitem_cast<zGraph *>(it);
        auto lwItem = listWidget->item(num);
        QFont font = lwItem->font();
        font.setBold(zitem->dockw->isVisible());
        lwItem->setFont(font);
        num++;
    }  // for
    view->winZGraphListAct->setChecked(dockZGraphList->isVisible());
    view->channelListAct->setChecked(dockChannelList->isVisible());


}

/*void QMainWindow::splitDockWidget ( QDockWidget * first, QDockWidget * second, Qt::Orientation orientation )*/

void MainWindow::createConstDockWidgets()
{
    dockZGraphList->setFloating(true);
    dockZGraphList->setFixedSize(220,200);
    dockZGraphList->setWidget(listWidget);
//    dockZGraphList->setWindowFlag(Qt::WindowCloseButtonHint, false);
    addDockWidget(Qt::BottomDockWidgetArea, dockZGraphList);
    connect(dockZGraphList->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
//    connect(dockZGraphList, SIGNAL(visibilityChanged()), this, SLOT(winZGraphList()));
    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listWidgetClicked(QListWidgetItem*)));
    connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(listWidgetDoubleClicked(QListWidgetItem*)));
    dockChannelList->setFloating(true);
    dockChannelList->setFixedSize(220,700);
    dockChannelList->setWidget(chListWidget);
    addDockWidget(Qt::BottomDockWidgetArea, dockChannelList);
    connect(dockChannelList->toggleViewAction(),SIGNAL(toggled(bool)),this,SLOT(toggleViewAction(bool)));
}

void MainWindow::open() {
  QString fname = QFileDialog::getOpenFileName(
      this, tr("Открытие файла данных"), "../data",
      tr("ENVI HDR Files (*.hdr);;ENVI HDR Files (*.hdr)"));
  emit read_file(fname);
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
  progress_dialog->hide();
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
    auto strlist = im_handler->getWaveLengthsList(2);
    foreach(QString str, strlist) {
        QListWidgetItem *lwItem = new QListWidgetItem();
        lwItem->setText(str);
        chListWidget->addItem(lwItem);
    }  // for
//    chListWidget->setItemSelected(chListWidget->item(0), true);
    chListWidget->setCurrentRow(0);
    connect(chListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClickedChannelList(QListWidgetItem*)));
}

void MainWindow::itemClickedChannelList(QListWidgetItem *lwItem)
{
    int num = chListWidget->row(lwItem);
    QImage img;
    if (num == 0) img = im_handler->get_rgb(true,60,53,12);
    else img = im_handler->get_rgb(true,num - 1,num - 1,num - 1);
    QPixmap pxm = QPixmap::fromImage(img);
    mainPixmap->setPixmap(pxm); // p->type(); enum { Type = 7 };
}

/*QListWidgetItem *lwItem = new QListWidgetItem();
    lwItem->setText(item->getTitle());
    QFont font = lwItem->font();  font.setBold(true);
    lwItem->setFont(font);
    lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
    lwItem->setCheckState(Qt::Checked);
    lwItem->setIcon(item->aicon);
//    listWidget->addItem(lwItem);
    listWidget->insertItem(0, lwItem);*/

void MainWindow::add_envi_hdr_pixmap() {
  QImage img = im_handler->get_rgb(true,60,53,12);
  QPixmap pxm = QPixmap::fromImage(img);
  mainPixmap = scene->addPixmap(pxm);  // p->type(); enum { Type = 7 };
  createDockWidgetForChannels();
}

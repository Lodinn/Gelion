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
  connect(view,SIGNAL(insertZGraphItem(zGraph*)),SLOT(createDockWidgetForItem(zGraph*)));
  connect(scene,SIGNAL(selectionChanged()),view,SLOT(selectionZChanged()));
  connect(view->winZGraphListAct, SIGNAL(triggered()), this, SLOT(winZGraphList()));
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
    lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
    lwItem->setCheckState(Qt::Checked);
    lwItem->setIcon(item->aicon);
//    listWidget->addItem(lwItem);
    listWidget->insertItem(0, lwItem);
}

void MainWindow::winZGraphList()
{
    dockZGraphList->setVisible(!dockZGraphList->isVisible());
}

void MainWindow::listWidgetClicked(QListWidgetItem *item)
{
    if (!view->zcRects.isEmpty()) return;
    int num = listWidget->row(item);
    QList<QGraphicsItem *> items = view->scene()->items();
    qDebug() << "listWidgetClicked";
    if (item->checkState() == Qt::Checked) items[num]->setVisible(true);
    else items[num]->setVisible(false);
}

void MainWindow::listWidgetDoubleClicked(QListWidgetItem *item)
{
    if (!view->zcRects.isEmpty()) return;
    int num = listWidget->row(item);
    QList<QGraphicsItem *> items = view->scene()->items();
    zGraph *it = qgraphicsitem_cast<zGraph *>(items[num]);
    zgraphParamDlg *dlg = new zgraphParamDlg(this);
    dlg->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
    dlg->ui->lineEditTitle->setText(it->getTitle());
    if (dlg->exec() == QDialog::Accepted) {
        QString str = dlg->ui->lineEditTitle->text();
        it->setTitle(str);
        item->setText(str);
    }  // if
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

}

void MainWindow::createStatusBar() {
    statusBar()->showMessage("Ctrl+Скролл-масштабирование*Скролл-перемотка верт.*Alt+Скролл-перемотка гориз.*Левая кн.мышь-перемещение");
}

void MainWindow::createConstDockWidgets()
{
    dockZGraphList->setFloating(true);
    dockZGraphList->setFixedSize(200,200);
    dockZGraphList->setWidget(listWidget);
    addDockWidget(Qt::BottomDockWidgetArea, dockZGraphList);
    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listWidgetClicked(QListWidgetItem*)));
    connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(listWidgetDoubleClicked(QListWidgetItem*)));
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

void MainWindow::add_envi_hdr_pixmap() {
  QImage img = im_handler->get_rgb(true);
  QPixmap pxm = QPixmap::fromImage(img);
  scene->addPixmap(pxm);  // p->type(); enum { Type = 7 };
}

#include "mainwindow.h"

#include <QMenu>
#include <QtWidgets>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{    
    SetupUi();
}

MainWindow::~MainWindow()
{
    delete scene;
    delete view;
    delete im_handler;
    delete worker_thread;
//    delete progress_dialog;
}

void MainWindow::SetupUi()
{
    setWindowTitle("gelion");
    const QIcon icon = QIcon::fromTheme("mainwindow", QIcon(":/icons/256_colors.png"));
    setWindowIcon(icon);
    resize(900,700);
    scene = new QGraphicsScene(this);
    view = new gQGraphicsView(scene);
    setCentralWidget(view);
    QPixmap test(QString(":/images/view.jpg"));  // &&&
    scene->addPixmap(test);  // &&&
    createActions();
    createStatusBar();
    im_handler->moveToThread(worker_thread);
    connect(this, SIGNAL(read_file(QString)), im_handler, SLOT (read_envi_hdr(QString)), Qt::DirectConnection);
    connect(im_handler, SIGNAL(numbands_obtained(int)), this, SLOT (show_progress(int)), Qt::DirectConnection);
    connect(im_handler, SIGNAL (finished()), this, SLOT (add_envi_hdr_pixmap()));
    connect(view, SIGNAL (point_picked(QPointF)), this, SLOT (show_profile(QPointF)));
}

void MainWindow::show_profile(QPointF point)
{
    int px, py;
    px = int(point.rx());  py = int(point.ry());
    QVector<QPointF> v = im_handler->get_profile(QPoint(px, py));
    if (v.length() == 0) return;
    QDockWidget *dock = new QDockWidget(tr("PLOt profile"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea
                          | Qt::BottomDockWidgetArea);
    dock->setFixedHeight(200);
    addDockWidget(Qt::BottomDockWidgetArea, dock);

    int d = im_handler->get_bands_count();
    qDebug() << point << "show_profile" << "d= " << d;
    QVector<double> x(d), y(d);
    for (int i=0; i<d; ++i)
    {
        x[i] = v[i].rx();
        y[i] = v[i].ry();
        qDebug() << "x[i]= " << x[i] << "y[i]= " << y[i];
    }
    wPlot= new QCustomPlot();
    dock->setWidget(wPlot);
    wPlot->resize(500,200);
    wPlot->addGraph();  wPlot->graph(0)->setData(x, y);
    // give the axes some labels:
    wPlot->xAxis->setLabel("длина волны, нм");    wPlot->yAxis->setLabel("коэффициент отражения");
    // set axes ranges, so we see all data:
    wPlot->xAxis->setRange(390, 1010);    wPlot->yAxis->setRange(0, 1.2);
}

void MainWindow::createActions()
{
    // file
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("Файл"));
    const QIcon openIcon = QIcon::fromTheme("Открыть файл...", QIcon(":/icons/open.png"));
    QAction *openAct = new QAction(openIcon, tr("&Открыть файл..."), this);
    QAction *closeAct = new QAction(tr("&Выход"), this);
    closeAct->setShortcuts(QKeySequence::Close);
    fileMenu->addAction(openAct);  connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAct);  connect(closeAct, &QAction::triggered, this, &MainWindow::close);
    fileToolBar->addAction(openAct);
    // edit
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Редактирование"));
    const QIcon printIcon = QIcon::fromTheme("Печать", QIcon(":/icons/butterfly.png"));
    QAction *printAct = new QAction(printIcon, tr("&Печать"), this);
    editMenu->addAction(printAct);
    editToolBar->addAction(printAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Строка статусных сообщений"));
 }

void MainWindow::open()
{  
    qDebug("open");
    QString fname = QFileDialog::getOpenFileName(this,
                                tr("Открытие файла данных"),
                                "../profile/data",
                                tr("ENVI HDR Files (*.hdr);;ENVI HDR Files (*.hdr)"));
    qDebug("fname");
    emit read_file(fname);
}

void MainWindow::show_progress(int max_progress)
{

    qDebug("show_progress");
//    QProgressDialog *progress_dialog = new QProgressDialog("Загрузка файла ...", "Отмена", 0, max_progress, this);
    qDebug() << progress_dialog;
    if (progress_dialog != nullptr) delete progress_dialog;
    progress_dialog  = new QProgressDialog("Загрузка файла ...", "Отмена", 0, 100, this);
    progress_dialog->resize(350, 100);
    progress_dialog->setMaximum(max_progress);
    progress_dialog->show();  QApplication::processEvents();
    connect(im_handler, SIGNAL (reading_progress(int)), progress_dialog, SLOT (setValue(int)));
    connect(im_handler, SIGNAL (finished()), this, SLOT (delete_progress_dialog()), Qt::DirectConnection);
    connect(progress_dialog, SIGNAL (canceled()), this, SLOT (stop_reading_file()), Qt::DirectConnection);

}

void MainWindow::stop_reading_file()
{
    qDebug("stop_reading_file");
    im_handler->set_read_file_canceled();
    progress_dialog->hide();  QApplication::processEvents();
}

void MainWindow::delete_progress_dialog()
{
    qDebug("delete_progress_dialog");
    if (progress_dialog != nullptr) { delete progress_dialog;
    progress_dialog = nullptr;}
    qDebug() << progress_dialog;
    //    delete progress_dialog;
}

void MainWindow::add_envi_hdr_pixmap()
{
    QImage img = im_handler->get_rgb(true);
    QPixmap pxm = QPixmap::fromImage(img);
    scene->addPixmap(pxm);
}

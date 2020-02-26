#include "mainwindow.h"

#include <QDockWidget>
#include <QGraphicsItem>
#include <QSettings>
#include <QDebug>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(700,500);
    view = new zView();
    setCentralWidget(view);
    QGraphicsRectItem *rect = view->scene()->addRect(-150,-150,300,250);
    rect->setFlag(QGraphicsItem::ItemIsMovable);
    rect->setFlag(QGraphicsItem::ItemIsSelectable);

    connect(view->dockAct, SIGNAL(triggered()), this, SLOT(createDock()));
    connect(view->closeAct, &QAction::triggered, this, &QMainWindow::close);
    connect(view->docksaveAct, SIGNAL(triggered()), this, SLOT(writeSettings()));
    connect(view->dockrestoreAct, SIGNAL(triggered()), this, SLOT(readSettings()));
}

MainWindow::~MainWindow()
{

}

void MainWindow::readSettings(){

    qDebug() << "readSettings";

    QSettings settings("MyCompany", "MyApp");
    QString str = settings.value("dockWidgets").toString();
    QStringList slst = str.split("~");
    foreach (QString str, slst)
        if (str.length() > 0) {
            QDockWidget *dock = new QDockWidget(str, this);
            dock->setObjectName(str);
        }
    QString strtbars = settings.value("toolBars").toString();
    QStringList stblst = strtbars.split("~");
    foreach (QString str, stblst)
        if (str.length() > 0) {
            QToolBar *fileToolBar = addToolBar("file");
            fileToolBar->setObjectName(str);
            fileToolBar->addAction(view->dockAct);
            fileToolBar->addAction(view->docksaveAct);
        }
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "closeEvent";
    QSettings settings("MyCompany", "MyApp");
    QList <QDockWidget * > dockWidgets = findChildren <QDockWidget * > ( ) ;
    QList<QToolBar*> toolbars = findChildren<QToolBar*>();
    QString str("~");
    foreach (QDockWidget *dw, dockWidgets) str.append(dw->objectName() + "~");
    settings.setValue("dockWidgets", str);
    QString strtbars("~");
    foreach (QToolBar *tb, toolbars) strtbars.append(tb->objectName() + "~");
    settings.setValue("toolBars", strtbars);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::writeSettings(){

    qDebug() << "writeSettings";

}

void MainWindow::createDock()
{
    qDebug() << "createDock";
    QDockWidget *dock = new QDockWidget(this);
    QString str = QString("dock number %1").arg(dnum);  dnum++;
    dock->setObjectName(str);
    dock->setWindowTitle(str);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    QToolBar *fileToolBar = addToolBar("file");
    fileToolBar->setObjectName("fff");
    fileToolBar->addAction(view->dockAct);
    fileToolBar->addAction(view->docksaveAct);

}

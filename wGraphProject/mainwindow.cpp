#include "mainwindow.h"

#include <QDockWidget>
#include <QGraphicsItem>
#include <QSettings>
#include <QDebug>
#include <QToolBar>
#include "../zgraphparamdlg.h"
#include "../ui_zgraphparamdlg.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(900,700);
    view = new zView();
    setCentralWidget(view);

    QAction *saveStateAct = new QAction("saveStateAct");
    QAction *restoreStateAct = new QAction("restoreStateAct");

    connect(view->saveStateAct, SIGNAL(triggered(bool)), this, SLOT(testSlotSaveApiState()));
    connect(view->restoreStateAct, SIGNAL(triggered(bool)), this, SLOT(testSlotRestoreApiState()));

    connect(view->dockAct, SIGNAL(triggered()), this, SLOT(createDock()));
    connect(view->closeAct, &QAction::triggered, this, &QMainWindow::close);
    connect(view->docksaveAct, SIGNAL(triggered()), this, SLOT(writeSettings()));
    connect(view->dockrestoreAct, SIGNAL(triggered()), this, SLOT(readSettings()));


    connect(itemslistdock, SIGNAL(visibilityChanged(bool)),this,SLOT(visibilityChangedDock(bool)));
    connect(view->visibleDock, SIGNAL(triggered()), this, SLOT(setVisibleAllDocks()));

    itemslistdock->setObjectName("itemslistdock");
    itemslistdock->setWindowTitle("Список графических объектов");
    itemslistdock->setFloating(true);
    itemslistdock->resize(500,200);
    itemslistdock->setMinimumWidth(120);
    itemsList = new QListWidget(itemslistdock);
    connect(itemsList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked(QListWidgetItem*)));
    itemsList->addItem("ggggggggg");
    itemslistdock->setWidget(itemsList);
    addDockWidget(Qt::LeftDockWidgetArea, itemslistdock);

    dock->setFeatures(dock->features() & ~QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();

    QDockWidget *d1 = new QDockWidget(this);
    d1->setWindowTitle("alex alex 5807 w1 w2");
    d1->setFloating(true);
    d1->setFixedSize(150,200);
    addDockWidget(Qt::NoDockWidgetArea, d1);
    QStringList strList;

    lw = new QListWidget(d1);


    QList<QGraphicsItem *> zGrItems = view->scene()->items();
    foreach ( QGraphicsItem *it , zGrItems ) strList << it->data(0).toString();

    const QIcon pointIcon = QIcon("G:/&&&proj/BPLA/images/pin_grey.png");
    lw->addItems(strList);
    d1->setWidget(lw);
    QListWidgetItem* item = 0;
    for(int i = 0; i < lw->count(); ++i) {
        item = lw->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        item->setIcon(pointIcon);
    }  // for

    connect(lw, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(lwItemClicked(QListWidgetItem*)));
    connect(lw, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(lwItemDblClicked(QListWidgetItem*)));

    qDebug() << dockWidgets;

    connect(view, SIGNAL(updateItemsList()), this, SLOT(updateDockItemsList()));
    emit view->updateItemsList();
}

void MainWindow::lwItemClicked(QListWidgetItem *item)
{
    int num = lw->row(item);
    QList<QGraphicsItem *> zGrItems = view->scene()->items();
    zGrItems[num]->setSelected(true);
    switch (item->checkState()) {
        case Qt::Unchecked : { zGrItems[num]->setVisible(false); return; }
    case Qt::PartiallyChecked : { zGrItems[num]->setVisible(true); return; }
    case Qt::Checked : { zGrItems[num]->setVisible(true); return; }
    }  // case
}

void MainWindow::lwItemDblClicked(QListWidgetItem *item)
{
    zgraphParamDlg *dlg = new zgraphParamDlg(this);
    dlg->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
//    const QIcon pointIcon = QIcon("G:/&&&proj/BPLA/images/pin_grey.png");
    QPixmap pxm = QPixmap("G:/&&&proj/BPLA/images/pin_grey.png");
    dlg->ui->label->setPixmap(pxm);
    dlg->ui->label->resize(pxm.size());

//    int num = lw->row(item);
//    QList<QGraphicsItem *> zGrItems = view->scene()->items();
//    zGraph *zgr = qgraphicsitem_cast<zGraph *>(zGrItems[num]);
//    QString title = zgr->getTitle();
    dlg->ui->lineEditTitle->setText("alex");
// В случае, если пользователь нажал "Ok".
    if (dlg->exec() == QDialog::Accepted) {
//        const QString &str1 = ledit1->text();
//        const QString &str2 = ledit2->text();
//        dlg->ui->lineEditTitle->setText("super title");
        QString str = dlg->ui->lineEditTitle->text();
        qDebug() << str;
    }
//
}

MainWindow::~MainWindow(){}

void MainWindow::testSlotSaveApiState(){

}

QPointF getPointFromStr(QString str) {
    QStringList pos = str.split(",");
    return QPointF(pos[0].toDouble(),pos[1].toDouble());
}

QVector<double> getVectorFromStr(QString str) {
    QStringList strlist = str.split(",");
    QVector<double> v;
    foreach(QString s, strlist) v.append(s.toDouble());
    return v;
}

void MainWindow::resetView()
{
//    view->resetMatrix();
//    view->resetTransform();
//    view->GlobalScale = 1.0;
//    view->GlobalRotate = 0.0;
    delete view;
    view = new zView();
    setCentralWidget(view);
}

void MainWindow::testSlotRestoreApiState(){
    qDebug() << "testSlotRestoreApiState";
// clear all dock widgets
    QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
    foreach(QDockWidget *dock, dockWidgets) {
        dock->setObjectName("");
        dock->setVisible(false);
        dock->deleteLater();
    }  // for

// clear graphics scene
    QList<QGraphicsItem *> items = view->scene()->items();
    foreach(QGraphicsItem *it, items) delete it;
    view->scene()->clear();
    createStaticDockWidgets();
    QSettings settings( "../settings_demo.ini", QSettings::IniFormat );
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    settings.setIniCodec( codec );

    int ver = settings.value("version").toInt();
    if (ver != 1) return;
    qreal scaleGlobal = settings.value("scale").toDouble();
    qreal rotationGlobal = settings.value("rotation").toDouble();
    resetView();
    view->scale(scaleGlobal,scaleGlobal);
    view->rotate(rotationGlobal);
    view->scene()->addPixmap(QPixmap("../wGraphProject/apple.jpg"));
    QStringList groups = settings.childGroups();
    qDebug() << groups;
//    return;
    foreach(QString str, groups) {
        if (str.indexOf("Rectangle") != -1) {
            settings.beginGroup(str);
            QString pos = settings.value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            zRect *rect = new zRect(pXY,scaleGlobal,rotationGlobal);
            QString title = settings.value("title").toString();
            rect->setTitle(title);
            int rot = settings.value("rotation").toInt();
            rect->setRotation( (qreal) rot );
            QString size = settings.value("size").toString();
            QPointF sWH = getPointFromStr(size);
            rect->frectSize.setWidth(sWH.rx());  rect->frectSize.setHeight(sWH.ry());
            rect->updateBoundingRect();
// create dynamic DockWidget
            view->scene()->addItem(rect);
            settings.endGroup();
            continue;
        }  // if Rectangle
        if (str.indexOf("Polygon") != -1) {
            settings.beginGroup(str);
            zPolygon *polygon = new zPolygon(scaleGlobal,rotationGlobal);
            QString title = settings.value("title").toString();
            polygon->setTitle(title);
            QString pos = settings.value("pos").toString();
            QPointF pXY = getPointFromStr(pos);
            polygon->setPos(pXY);
            QString x = settings.value("x").toString();
            QString y = settings.value("y").toString();
            auto vx = getVectorFromStr(x);
            auto vy = getVectorFromStr(y);
            for (int i = 0; i < vx.length(); i++) {
                polygon->fpolygon << QPoint((int) vx[i], (int) vy[i]);
            }  // for
            polygon->updateBoundingRect();
// create dynamic DockWidget
            view->scene()->addItem(polygon);
            settings.endGroup();
            continue;
        }  // if Polygon
    }  // for groups

//    restoreGeometry(settings.value("geometry").toByteArray());
//    restoreState(settings.value("windowState").toByteArray());
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

void MainWindow::updateDockItemsList()
{
    QList<QGraphicsItem *> items = view->scene()->items();
    itemsList->clear();
    foreach ( QGraphicsItem *it , items ) itemsList->addItem(it->data(0).toString());
}

void MainWindow::itemClicked(QListWidgetItem *item)
{
    int rw = itemsList->row(item);
    qDebug() << "rw=  " << rw;
    QList<QGraphicsItem *> items = view->scene()->items();
    if (items[rw]->isSelected()) items[rw]->setSelected(false);
    else items[rw]->setSelected(true);
}

void MainWindow::visibilityChangedDock(bool visible)
{
    qDebug() << "visibilityChangedDock" << visible;
    QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
    qDebug() << dockWidgets;
}

void MainWindow::setVisibleAllDocks()
{
    QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
    foreach(QDockWidget *d, dockWidgets)
        d->setVisible(true);
}

void MainWindow::createStaticDockWidgets()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
//    QSettings settings("MyCompany", "MyApp");
//    QList <QDockWidget * > dockWidgets = findChildren <QDockWidget * > ( ) ;
//    QList<QToolBar*> toolbars = findChildren<QToolBar*>();
//    QString str("~");
//    foreach (QDockWidget *dw, dockWidgets) str.append(dw->objectName() + "~");
//    settings.setValue("dockWidgets", str);
//    QString strtbars("~");
//    foreach (QToolBar *tb, toolbars) strtbars.append(tb->objectName() + "~");
//    settings.setValue("toolBars", strtbars);
//    settings.setValue("geometry", saveGeometry());
//    settings.setValue("windowState", saveState());
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

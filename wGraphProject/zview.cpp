#include "zview.h"

#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QSettings>

zView::zView(QWidget *parent)
    : QGraphicsView(parent)
{
    this->setScene(new QGraphicsScene());  // Устанавливаем сцену в виджет
    const QIcon i1 = QIcon("../images/pin2_red.png");
    const QIcon i2 = QIcon("../images/vector_ellipse.png");
    const QIcon i3 = QIcon("../images/vector_square.png");
    const QIcon i4 = QIcon("../images/polyline-64.png");
    const QIcon i5 = QIcon("../images/vector-polygon.png");
    testAct = new QAction(i1, "test", this);
    dockAct = new QAction(i2, "dock", this);
    closeAct = new QAction(i3, "close", this);
    docksaveAct = new QAction(i4, "docksaveAct", this);
    dockrestoreAct = new QAction(i5, "dockrestoreAct", this);
//    connect(testAct, &QAction::triggered, this, &gQGraphicsView::testSLOT);
    connect(testAct, SIGNAL(triggered()), this, SLOT(testSlot()));
}

zView::~zView()
{

}

void zView::testSlot()
{
    qDebug() << "testSlot";
//    QSettings settings( "settings_demo.conf", QSettings::IniFormat );
//    settings.beginGroup( "WidgetPosition" );
//    settings.setValue( "x", 100 );
//    settings.setValue( "y", 120 );
//    settings.setValue( "width", 77 );
//    settings.setValue( "height", 78 );
//    settings.endGroup();

//    settings.beginGroup( "WidgetContent" );
//    settings.setValue( "text", "edit.toPlainText()" );
//    settings.endGroup();

    QSettings settings( "settings_demo.conf", QSettings::IniFormat );
    settings.beginGroup( "WidgetPosition" );
    int x = settings.value( "x", -1 ).toInt();
    int y = settings.value( "y", -1 ).toInt();
    int width = settings.value( "width", -1 ).toInt();
    int height = settings.value( "height", -1 ).toInt();
    settings.endGroup();
    qDebug() << x << y << width << height;
    settings.beginGroup( "WidgetContent" );
    QString text = settings.value( "text", "" ).toString();
    settings.endGroup();
    qDebug() << text;
}

void zView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(dockAct);
    menu.addAction(testAct);
    menu.addAction(docksaveAct);
    menu.addAction(dockrestoreAct);
    menu.addSeparator();
    menu.addAction(closeAct);
    menu.exec(event->globalPos());
}

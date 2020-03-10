#include "zview.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QSettings>
#include <QtMath>

zView::zView(QWidget *parent)
    : QGraphicsView(parent)
{
    this->setScene(new QGraphicsScene());  // Устанавливаем сцену в виджет
    //----------------------------------------------------- main
    connect(scene(),SIGNAL(selectionChanged()),this,SLOT(createGrabberRects()));
    pointAct = new QAction(QIcon("../images/pin2_red.png"), "введите точку построения профиля", this);
    polygonAct = new QAction(QIcon("../images/vector-polygon.png"), "введите область построения профиля", this);
    rectAct = new QAction(QIcon("../images/vector_square.png"), "введите rect построения профиля", this);

    connect(pointAct, SIGNAL(triggered()), this, SLOT(setInputModePoint()));
    connect(polygonAct, SIGNAL(triggered()), this, SLOT(setInputModePolygon()));
    connect(rectAct, SIGNAL(triggered()), this, SLOT(setInputModeRect()));
    //----------------------------------------------------- main

    const QIcon i2 = QIcon("../images/vector_ellipse.png");
    const QIcon i3 = QIcon("../images/vector_square.png");
    const QIcon i4 = QIcon("../images/polyline-64.png");
    const QIcon i5 = QIcon("../images/vector-polygon.png");
    testAct = new QAction(QIcon("../images/polyline-64.png"), "test", this);
    dockAct = new QAction(i2, "dock", this);
    closeAct = new QAction(i3, "close", this);
    docksaveAct = new QAction(i4, "docksaveAct", this);
    dockrestoreAct = new QAction(i5, "dockrestoreAct", this);

    connect(testAct, SIGNAL(triggered()), this, SLOT(testSlot()));
//  =============== ввод графических объектов


    setSelAct = new QAction("выбранный объект вручную", this);
    connect(setSelAct, SIGNAL(triggered()), this, SLOT(setSelectionForce()));

    zPolygon *zp = new zPolygon();
    zp->addPoint(QPoint(50,-100));zp->addPoint(QPoint(200,30));
    zp->addPoint(QPoint(170,170));zp->addPoint(QPoint(100,170));
    zp->addPoint(QPoint(0,170));zp->addPoint(QPoint(-20,120));
    zp->setTitle("Полигон 1");
    zp->updateBoundingRect();
    scene()->addItem(zp);

    zRect *rect = new zRect(QPointF(0,-100));
    rect->setTitle("rect 1");
    rect->frectSize = QSize(200,50);
    rect->updateBoundingRect();
    scene()->addItem(rect);

    emit updateItemsList();


}

zView::~zView(){}

void zView::testSlot()
{
    qDebug() << "testSlot";

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

void zView::setInputModePoint()
{
    insertMode = zView::Point;
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    showMessageWidget("input point by click");
}

void zView::setInputModePolygon()
{
    insertMode = zView::Polygon;
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

void zView::setInputModeRect()
{
    insertMode = zView::Rect;
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    showMessageWidget("Нажмите клавишу мыши\nШирину выбирайте колесом мыши");
}

void zView::setSelectionForce()
{

}

void zView::setGrabberCoordTozRect()
{
    zRect *zR = qgraphicsitem_cast<zRect *>(grabberItem);
    QPointF pos = zR->pos();
    qreal w = zR->frectSize.width();
    qreal degrees = zR->rotation();
    double radians = qDegreesToRadians(degrees);
    QPointF pos2 = QPointF(pos.rx()+w*qCos(radians),pos.ry()+w*qSin(radians));
    zcRects[0]->setPos(pos);
    zcRects[1]->setPos(pos2);
}

void zView::createGrabberRects()
{
    deleteGrabberRects();
    grabberItem = qgraphicsitem_cast<zGraph *>(scene()->mouseGrabberItem());
    if (grabberItem == 0) return;
    switch (grabberItem->type()) {
    case zRect::Type : {        // тип объекта - прямоугольник Rect 2	с возможностью поворота
        grabberItem->setCursor(QCursor(Qt::SizeAllCursor));
        connect(grabberItem,SIGNAL(mouseMove()),this,SLOT(moveGrabberRects()));
        zRect *zR = qgraphicsitem_cast<zRect *>(grabberItem);
        QPointF pos = zR->pos();
        zContourRect *zrect = new zContourRect(pos);  // первая точка (0)
        connect(zrect,SIGNAL(mousePress()),this,SLOT(pressOneGrabberRect()));
        connect(zrect,SIGNAL(mouseMove(int,QPointF)),this,SLOT(moveOneGrabberRect(int,QPointF)));
        connect(zrect,SIGNAL(mouseRelease()),this,SLOT(releaseOneGrabberRect()));
        zrect->setData(0, 0);  scene()->addItem(zrect); zcRects.append(zrect);
        zrect = new zContourRect(pos);              // вторая точка (1)
        zrect->setData(0, 1);  scene()->addItem(zrect); zcRects.append(zrect);
        setGrabberCoordTozRect();
        connect(zrect,SIGNAL(mousePress()),this,SLOT(pressOneGrabberRect()));
        connect(zrect,SIGNAL(mouseMove(int,QPointF)),this,SLOT(moveOneGrabberRect(int,QPointF)));
        connect(zrect,SIGNAL(mouseRelease()),this,SLOT(releaseOneGrabberRect()));
        break;
    }  // case zRect::Type
    case zPolygon::Type : {             // тип объекта - область(полигон)  Polygon       5
        grabberItem->setCursor(QCursor(Qt::SizeAllCursor));
        connect(grabberItem,SIGNAL(mouseMove()),this,SLOT(moveGrabberRects()));
        zPolygon *zp = qgraphicsitem_cast<zPolygon *>(grabberItem);
        foreach (QPoint point, zp->fpolygon) {
            QPointF pos = zp->pos();
            QPointF zpoint(pos.rx()+point.rx(), pos.ry()+point.ry());
            zContourRect *zrect = new zContourRect(zpoint);
            zrect->setData(0, zp->fpolygon.indexOf(point));
            scene()->addItem(zrect);
            zcRects.append(zrect);
            connect(zrect,SIGNAL(mousePress()),this,SLOT(pressOneGrabberRect()));
            connect(zrect,SIGNAL(mouseMove(int,QPointF)),this,SLOT(moveOneGrabberRect(int,QPointF)));
            connect(zrect,SIGNAL(mouseRelease()),this,SLOT(releaseOneGrabberRect()));
        }
        break;
    }  // case zPolygon::Type
    default:
        break;
    }  // switch

}

void zView::deleteGrabberRects()
{
    foreach (zContourRect *rect, zcRects) {
        rect->disconnect(); scene()->removeItem(rect);
        delete rect;        rect = nullptr;
    } zcRects.clear();
    if (grabberItem == 0) return;
    grabberItem->setCursor(QCursor(Qt::ArrowCursor));
    grabberItem->disconnect();
}

void zView::moveGrabberRects()
{
    if (grabberItem == 0) return;
    switch (grabberItem->type()) {
    case zRect::Type : {        //- прямоугольник     Rect          2	с возможностью поворота
        setGrabberCoordTozRect();
        break;
    }  // case zRect::Type
    case zPolygon::Type : {             // тип объекта - область(полигон)  Polygon       5
        zPolygon *zp = qgraphicsitem_cast<zPolygon *>(grabberItem);
        foreach (QPoint point, zp->fpolygon) {
            QPointF pos = zp->pos();
            zcRects[zp->fpolygon.indexOf(point)]->setPos(pos.rx()+point.rx(), pos.ry()+point.ry());
           }
        break;
    }  // case zPolygon::Type
    default:
        break;
    }  // switch
}

void zView::pressOneGrabberRect()
{
    grabberItem->setFlag(QGraphicsItem::ItemIsMovable, false);
}

void zView::moveOneGrabberRect(int num, QPointF point)
{
    switch (grabberItem->type()) {
    case zRect::Type : {    // тип объекта - прямоугольник Rect 2	с возможностью поворота
        zRect *zR = qgraphicsitem_cast<zRect *>(grabberItem);
        if (num == 0) {
            zR->setPos(point);
            QPointF delta = zcRects[1]->pos() - zcRects[0]->pos();
            QLineF line(zcRects[1]->pos(),zcRects[0]->pos());
            qreal angle = qAtan2(delta.ry(),delta.rx()) * 180 / 3.1415;
            zR->setRotation(angle);
            zR->frectSize.setWidth(line.length());
            zR->updateBoundingRect();
            zR->update();
        }  // if (num == 0)
        if (num == 1) {
            QPointF delta = zcRects[1]->pos() - zcRects[0]->pos();
            QLineF line(zcRects[1]->pos(),zcRects[0]->pos());
            qreal angle = qRadiansToDegrees(qAtan2(delta.ry(),delta.rx()));
            zR->setRotation(angle);
            zR->frectSize.setWidth(line.length());
            zR->updateBoundingRect();
            zR->update();
        }  // if (num == 1)
        break;
    }  // case zRect::Type
    case zPolygon::Type : {             // тип объекта - область(полигон)  Polygon       5
        zPolygon *zp = qgraphicsitem_cast<zPolygon *>(grabberItem);
        QPointF pos = zp->pos();
        zp->fpolygon[num].setX(point.rx() - pos.rx());
        zp->fpolygon[num].setY(point.ry() - pos.ry());
        zp->updateBoundingRect();
        zp->update();
        break;
    }  // case zPolygon::Type
    default:
        break;
    }  // switch
}

void zView::releaseOneGrabberRect()
{
    grabberItem->setFlag(QGraphicsItem::ItemIsMovable, true);
}

void zView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!contextMenuEnable) {
        contextMenuEnable = true;  return;
    }
    QMenu menu(this);
    menu.addAction(pointAct);
    menu.addAction(polygonAct);
    menu.addAction(rectAct);
    menu.addSeparator();
    menu.addAction(dockAct);
    menu.addAction(testAct);
    menu.addAction(docksaveAct);
    menu.addAction(dockrestoreAct);
    menu.addSeparator();
    menu.addAction(closeAct);
    contextPos = mapToScene(event->pos());
    menu.exec(event->globalPos());
}

void zView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    switch (insertMode) {
    case zView::None : {
        break;
    }  // case zView::None
    case zView::Point : {
        if (event->button() != Qt::LeftButton) return;
        createPointGraphItem(event->pos());
        insertMode = zView::None;
        setCursor(Qt::ArrowCursor);
        setMouseTracking(false);
        break;
    }  // case zView::Point
    case zView::Rect : {
        if (event->button() != Qt::LeftButton) return;
        if (tmpLines.isEmpty()) {
            QPointF p = mapToScene(event->pos());
            qreal x1 = p.rx();  qreal y1 = p.ry();
            qreal x2 = p.rx();  qreal y2 = p.ry();
            tmpLines.append(scene()->addLine(x1,y1,x2,y2,fInsPen));
            tmpRect = new zRect(p);
            tmpRect->updateBoundingRect();
            scene()->addItem(tmpRect);
            break;
        } else {
            tmpRect->setTitle("Прямоугольник 1");
            tmpRect->updateBoundingRect();
            createRect();
            insertMode = zView::None;
            setCursor(Qt::ArrowCursor);
            setMouseTracking(false);
            break;
        }  // if (tmpLines.isEmpty())
    }  // case zView::Rect
    case zView::Ellipse : {

    }  // case zView::Ellipse
    case zView::Polyline : {

    }  // case zView::Polyline
    case zView::Polygon : {
        contextMenuEnable = false;
        if (event->button() == Qt::LeftButton) {
            QPointF p = mapToScene(event->pos());
            qreal x1 = p.rx();  qreal y1 = p.ry();
            qreal x2 = p.rx();  qreal y2 = p.ry();
            tmpLines.append(scene()->addLine(x1,y1,x2,y2,fInsPen));
            break;
        };  // Qt::LeftButton
        if (event->button() == Qt::RightButton) {
            createPolygon();
            insertMode = zView::None;
            setCursor(Qt::ArrowCursor);
            setMouseTracking(false);
            break;
        };  // Qt::RightButton
        return;
    }  // case zView::Polygon
    default:
        break;
    }
}

void zView::mouseMoveEvent(QMouseEvent *event)
{
    switch (insertMode) {
    case zView::Rect : {
        if (tmpRect == nullptr) return;
        if (tmpLines.isEmpty()) return;
        QPointF point = mapToScene(event->pos());
        QLineF newLine(tmpLines[0]->line().p1(), point);
        tmpLines[0]->setLine(newLine);
        QPointF p1 = newLine.p1();
        qreal dx  = point.x() - p1.x();
        qreal dy  = point.y() - p1.y();
        qreal angle = qAtan2(dy,dx) * 180 / 3.1415;
        tmpRect->setRotation(angle);
        qreal w = newLine.length();
        tmpRect->frectSize.setWidth(w);
        tmpRect->updateBoundingRect();
        tmpRect->update();
    }  // case zView::Rect
    case zView::Polygon : {
        if (tmpLines.isEmpty()) return;
        QGraphicsLineItem *line = tmpLines[tmpLines.length()-1];
        QLineF newLine(line->line().p1(), mapToScene(event->pos()));
        line->setLine(newLine);
        return;
    }  // case zView::Polygon
    default:
        break;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void zView::createPointGraphItem(QPoint pos)
{
    QPointF point = mapToScene(pos);
    zPoint *zp = new zPoint(point);
    zp->setTitle("Точка 1");
    zp->updateBoundingRect();
    scene()->addItem(zp);
    if (w != nullptr) { scene()->removeItem(w);  w = nullptr; }
    emit updateItemsList();
}

void zView::wheelEvent(QWheelEvent *event)
{
    QList<QGraphicsItem *> selection = scene()->selectedItems();
    if (selection.length() > 0) {
        zGraph *graph  = qgraphicsitem_cast<zGraph *>(selection[0]);
        switch (graph->type()) {
        case zRect::Type : {        // тип объекта - прямоугольник Rect 2	с возможностью поворота
            zRect *zR = qgraphicsitem_cast<zRect *>(graph);
            qreal h = 0.0;
            if(event->delta() > 0) h = 2.0;
            else h = - 2.0;
            qreal newheight = zR->frectSize.height()+h;
            if (newheight >= 0.0)
                zR->frectSize.setHeight(newheight);
            zR->updateBoundingRect();
            zR->update();
            return;
        }  // case zRect::Type
        }  // switch (graph->type())
    }  // if (selection.length() > 0)
    switch (insertMode) {
    case zView::Rect : {
        if (tmpRect == nullptr) break;
        qreal h = 0.0;
        if(event->delta() > 0) h = 2.0;
        else h = - 2.0;
        qreal newheight = tmpRect->frectSize.height()+h;
        if (newheight >= 0.0)
            tmpRect->frectSize.setHeight(newheight);
        tmpRect->updateBoundingRect();
        tmpRect->update();
        return;
    }  // case zView::Rect
    }  // switch (insertMode)
    if (event->modifiers() == Qt::ControlModifier) {
        qreal sx;
        if(event->delta() > 0) sx = 1.05;
        else sx = 0.95;
        scale(sx, sx);  zScale /= sx;
        setScaleAngle(zScale,zAngle);
        return; }
    if (event->modifiers() == Qt::ShiftModifier) {
        qreal angle;
        if(event->delta() > 0) angle = 1.0;
        else angle = -1.0;
        rotate(angle);  zAngle -= angle;
        setScaleAngle(zScale,zAngle);
        return; }
    QGraphicsView::wheelEvent(event);
}

void zView::setScaleAngle(qreal sc, qreal an)
{
    QList<QGraphicsItem *> items = scene()->items();
    foreach ( QGraphicsItem *it , items ) {
        it->setData(1,sc);
        it->setData(2,an);
        if (items.indexOf(it) > 1)
            qgraphicsitem_cast<zGraph *>(it)->updateBoundingRect();
    }
}

void zView::createPolygon()
{
    zPolygon *zp = new zPolygon();
    zp->setTitle("Полигон 1");
    foreach (QGraphicsLineItem *item, tmpLines) {
        QPointF point = item->line().p1();
        zp->fpolygon.append(point.toPoint());
    }
    zp->fpolygon.append(tmpLines[tmpLines.length()-1]->line().p2().toPoint());
    zp->updateBoundingRect();
    scene()->addItem(zp);
    foreach (QGraphicsLineItem *item, tmpLines) {
        scene()->removeItem(item);
        delete item;
    }
    tmpLines.clear();
    if (w != nullptr) { scene()->removeItem(w);  w = nullptr; }
}

void zView::createRect()
{
    tmpRect = nullptr;
    foreach (QGraphicsLineItem *item, tmpLines) {
        scene()->removeItem(item);
        delete item;
    }
    tmpLines.clear();
    if (w != nullptr) { scene()->removeItem(w);  w = nullptr; }
}

void zView::showMessageWidget(QString str)
{
    QWidget *wgt = new QWidget();
    wgt->resize(100,100);
    QLabel *label = new QLabel();
    QHBoxLayout *layout = new QHBoxLayout();
    label->setText(str);
    layout->addWidget(label);
    wgt->setLayout(layout);
    wgt->setGeometry(contextPos.x(),contextPos.y(),100,20);
    wgt->setWindowOpacity(0.5);
    w = scene()->addWidget(wgt);
    w->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}

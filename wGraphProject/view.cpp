#include "view.h"

#include <QGraphicsScene>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QGraphicsItem>
#include <QDebug>
#include <QMenu>
#include <QtMath>

wview::wview(QGraphicsScene *scene)
    : QGraphicsView(scene)
{
    createActions();
    connect(insEllipseItemAct, &QAction::triggered, this, &wview::insEllipseItemSLOT);
    connect(insPolylineItemAct, &QAction::triggered, this, &wview::insPolylineItemSLOT);
    connect(testAct, &QAction::triggered, this, &wview::testSLOT);
}

void wview::insEllipseItemSLOT()
{
    insertMode = gQGraphicsView::Ellipse;
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    tmpEllipse = nullptr;
}

void wview::insPolylineItemSLOT()
{
    insertMode = gQGraphicsView::Polyline;
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    tmpLine = nullptr;
}

void wview::testSLOT()
{
    wEllipse *ellipse = new wEllipse();
    ellipse->setPos(-450,-200);
    ellipse->width = 200;
    ellipse->height = 100;
    ellipse->updateBoundingRect();
    ellipse->setFlag(QGraphicsItem::ItemIsMovable);
    ellipse->setFlag(QGraphicsItem::ItemIsSelectable);
    scene()->addItem(ellipse);
    QRectF rect = ellipse->boundingRect();
    for (int i=rect.left(); i<rect.width(); i++)
        for (int j=rect.top(); j<rect.height(); j++)
            if (ellipse->contains(QPointF(i, j))) {
                int x = ellipse->pos().x() + i;
                int y = ellipse->pos().y() + j;
                scene()->addRect(x, y, 2, 2, QPen(Qt::red));
            }
}

void wview::contextMenuEvent(QContextMenuEvent *event)
{
    if (contextMenuTrigger == 1) {
        contextMenuTrigger = 0;
        return;
    }
    QMenu menu(this);
    menu.addAction(insPointItemAct);
    menu.addAction(insEllipseItemAct);
    menu.addAction(insRectItemAct);
    menu.addAction(insPolylineItemAct);
    menu.addAction(insPolygonItemAct);
    menu.addAction(closeAct);
    menu.addAction(testAct);
    menu.exec(event->globalPos());
}

void wview::addwEllipseToScene()
{
    wEllipse *ellipse = new wEllipse();
    ellipse->setTransformOriginPoint(tmpEllipse->transformOriginPoint());
    ellipse->setPos(tmpEllipse->pos());
    ellipse->setRotation(tmpEllipse->rotation());
    QRectF rect = tmpEllipse->rect();
    ellipse->width = rect.width();
    ellipse->height = rect.height();
    ellipse->updateBoundingRect();
    ellipse->setFlag(QGraphicsItem::ItemIsMovable);
    ellipse->setFlag(QGraphicsItem::ItemIsSelectable);
    scene()->addItem(ellipse);

    delete tmpEllipse;
    delete tmpLine;
    insertMode = gQGraphicsView::None;
    setCursor(Qt::ArrowCursor);
    setMouseTracking(false);
    tmpEllipse = nullptr;
}

void wview::addwPolylineToScene()
{
    wPolyline *wpoly = new wPolyline();
    qDebug() << "tmpPoints count = " << tmpPoints.count();
    foreach ( QPointF p, tmpPoints ) wpoly->polyline.append(p);
    wpoly->updateBoundingRect();
    wpoly->setFlag(QGraphicsItem::ItemIsMovable);
    wpoly->setFlag(QGraphicsItem::ItemIsSelectable);
    scene()->addItem(wpoly);
    foreach ( QGraphicsLineItem *tmpline, tmpLineItems ) scene()->removeItem(tmpline);
    tmpLineItems.clear();
    tmpPoints.clear();
    delete tmpLine;
    insertMode = gQGraphicsView::None;
    setCursor(Qt::ArrowCursor);
    setMouseTracking(false);
    contextMenuTrigger = 1;
}

void wview::wheelEvent(QWheelEvent *event)
{
    switch (insertMode) {
    case gQGraphicsView::Ellipse : {
        qreal h = 0.0;
        if(event->delta() > 0)
            h = tmpEllipse->rect().height() + 3.0;
        else
            h = tmpEllipse->rect().height() - 3.0;
        qreal w = tmpEllipse->rect().width();
        tmpEllipse->setRect(0, 0, w, h);
        tmpEllipse->setTransformOriginPoint(0, h/2);
        tmpEllipse->setPos(tmpLine->line().p1().rx(), tmpLine->line().p1().ry() - h/2);
        return;
    }
    }  // switch
    if (event->modifiers() == Qt::ControlModifier) {
        if(event->delta() > 0)
            scale(1.05, 1.05);
        else
            scale(0.95, 0.95);
        return; }
    if (event->modifiers() == Qt::ShiftModifier) {
        if(event->delta() > 0)
            rotate(1.0);
        else
            rotate(-1.0);
        return; }
    QGraphicsView::wheelEvent(event);
}

void wview::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    if (scene()->mouseGrabberItem() != nullptr) return;  // выбрана ОИ
    switch (insertMode) {
    case gQGraphicsView::None : {
        if (event->button() == Qt::LeftButton)  {
            _pan = true;
            _panX0 = event->x();
            _panY0 = event->y();
            setCursor(Qt::ClosedHandCursor);
            event->accept();
        }
        return;
    }
    case gQGraphicsView::Ellipse : {
        if (event->button() != Qt::LeftButton) return;
        if (tmpEllipse == nullptr) {
            QPointF point = mapToScene(event->pos());
            tmpEllipse = new QGraphicsEllipseItem(0,0,0,50);
            tmpEllipse->setTransformOriginPoint(0,50/2);
            tmpEllipse->setPos(point.rx(), point.ry()-50/2);
            scene()->addItem(tmpEllipse);
            tmpLine = new QGraphicsLineItem(QLineF(point, point));
            tmpLine->setPen(QPen(Qt::red, 3));
            scene()->addItem(tmpLine);
        } else {
            addwEllipseToScene();

        }
        return;
    }
    case gQGraphicsView::Polyline : {
        if (event->button() == Qt::RightButton) {
            QPointF point = mapToScene(event->pos());
            tmpPoints.append(point);
            addwPolylineToScene();
            return;
        }
        if (event->button() != Qt::LeftButton) return;
        if (tmpLine == nullptr) {
            QPointF point = mapToScene(event->pos());
            tmpLine = new QGraphicsLineItem(QLineF(point, point));
            tmpLine->setPen(QPen(Qt::red, 3));
            scene()->addItem(tmpLine);
            tmpLineItems.append(tmpLine);
            tmpPoints.append(point);
        } else {
            QPointF point = mapToScene(event->pos());
            QLineF newLine(tmpLine->line().p2(), point);
            tmpLine = new QGraphicsLineItem(newLine);
            tmpLine->setPen(QPen(Qt::red, 3));
            scene()->addItem(tmpLine);
            tmpLineItems.append(tmpLine);
            tmpPoints.append(point);
        }
        return;
    }
    default:
        break;
    }
}

void wview::mouseMoveEvent(QMouseEvent *event)
{
    switch (insertMode) {
    case gQGraphicsView::None : {
        if (_pan) {
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->x() - _panX0));
            verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->y() - _panY0));
            _panX0 = event->x();
            _panY0 = event->y();
            event->accept();
            return;
        }
        QGraphicsView::mouseMoveEvent(event);
        return;
    }
    case gQGraphicsView::Ellipse : {
       if (tmpEllipse == nullptr) return;
       QPointF point = mapToScene(event->pos());
       QLineF newLine(tmpLine->line().p1(), point);
       tmpLine->setLine(newLine);
       QPointF p1 = tmpLine->line().p1();
       qreal dx  = point.x() - p1.x();
       qreal dy  = point.y() - p1.y();
       qreal angle = qAtan2(dy,dx) * 180 / 3.1415;
       tmpEllipse->setRotation(angle);
       qreal h = tmpEllipse->rect().height();
       qreal w = tmpLine->line().length();
       tmpEllipse->setRect(0,0,w,h);
       tmpEllipse->setTransformOriginPoint(0,h/2);
       tmpEllipse->update();
       return;
    }
    case gQGraphicsView::Polyline : {
        if (tmpLine == nullptr) return;
        QPointF point = mapToScene(event->pos());
        QLineF newLine(tmpLine->line().p1(), point);
        tmpLine->setLine(newLine);
        return;
    }
    default:
        break;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void wview::mouseReleaseEvent(QMouseEvent *event)
{
    switch (insertMode) {
    case gQGraphicsView::None : {
        if (_pan) {
            _pan = false;
            setCursor(Qt::ArrowCursor);
            event->accept();
            return;
        }
    }
    default:
        break;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

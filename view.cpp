#include "view.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

ROI::ROI(QPointF position, QString l, shape_t sh)
    : shape(sh), pos(position), label(l) {
  setFlag(ItemIsSelectable, true);
  setFlag(ItemIsMovable);
}

QRectF ROI::boundingRect() const {
  switch (shape) {
    case Point:
      return QRectF(0, 0, 6, 6);
    // FIXME: not implemented
    case Line:
    case Circle:
    case Rectangle:
    case Polygon:
      return QRectF(0, 0, 10, 10);
  }
}

void ROI::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget) {
  QRectF rect = boundingRect();
  painter->translate(pos);
  QPen pen(Qt::red, 6);
  painter->setFont(QFont("Arial", 12));
  painter->drawText(20, 20, label);
  painter->setPen(pen);
  switch (shape) {
    case Point:
    case Circle:
      painter->drawEllipse(rect);
      break;
    case Line:
    case Rectangle:
    case Polygon:
      painter->drawRect(rect);
      break;
  }
  Q_UNUSED(option)
  Q_UNUSED(widget)
}

void ROI::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mousePressEvent(event);
}

void ROI::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mouseMoveEvent(event);
}

void ROI::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mouseReleaseEvent(event);
}

gQGraphicsView::gQGraphicsView(QGraphicsScene *scene) : QGraphicsView(scene) {}

void gQGraphicsView::wheelEvent(QWheelEvent *event) {
  if (event->modifiers() == Qt::ControlModifier) {
    if (event->delta() > 0)
      scale(1.2, 1.2);
    else
      scale(0.8, 0.8);
    return;
  }
  if (event->modifiers() == Qt::ShiftModifier) {
    if (event->delta() > 0)
      rotate(7.0);
    else
      rotate(-7.0);
    return;
  }
  QGraphicsView::wheelEvent(event);
}

void gQGraphicsView::mousePressEvent(QMouseEvent *event) {
  QGraphicsView::mousePressEvent(event);
  qDebug() << scene()->mouseGrabberItem();
  if (scene()->mouseGrabberItem() != nullptr) {
    qDebug() << "Grabbed something";
    QGraphicsView::mousePressEvent(event);
    return;
  }

  if (event->modifiers() == Qt::ControlModifier)
    if (event->button() == Qt::LeftButton) {
      QPointF point = mapToScene(event->pos());
      /*ROI *new_roi =
          new ROI(point, QString("Point %1").arg(ROIs.count() + 1), ROI::Point);
      scene()->addItem(new_roi);
      ROIs_new.append(new_roi);*/
      qDebug() << point;
      //            scene()->addRect(point.x(), point.y(), 36, 18);
      QPainterPath pp;
      pp.moveTo(point);
      pp.addEllipse(point, 6, 6);
      pp.addText(point.x(), point.y(), QFont("Arial", 12),
                 QString("Point %1").arg(ROIs.count() + 1));
      QGraphicsPathItem *path =
          scene()->addPath(pp, QPen(Qt::yellow), QBrush(Qt::yellow));
      path->setFlag(QGraphicsItem::ItemIsMovable);
      //      path->setFlag(QGraphicsItem::ItemIgnoresTransformations);
      ROIs.append(static_cast<QGraphicsItem *>(path));
      emit point_picked(point);
      return;
    }

  if (event->button() == Qt::LeftButton) {
    _pan = true;
    _panX0 = event->x();
    _panY0 = event->y();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
    return;
  }

  //    event->ignore();
}

void gQGraphicsView::mouseMoveEvent(QMouseEvent *event) {
  if (_pan) {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() -
                                    (event->x() - _panX0));
    verticalScrollBar()->setValue(verticalScrollBar()->value() -
                                  (event->y() - _panY0));
    _panX0 = event->x();
    _panY0 = event->y();
    event->accept();
    return;
  }
  qDebug() << "grabbing" << scene()->mouseGrabberItem();
  if (scene()->mouseGrabberItem() != nullptr) {
    qDebug() << mapToScene(event->pos()) << scene()->mouseGrabberItem()
             << ROIs_new.count();
    for (int i = 0; i < ROIs.count(); i++) {
      if (ROIs[i] == scene()->mouseGrabberItem()) {
        emit roi_position_updated(
            QPair<int, QPointF>(i, mapToScene(event->pos())));
      }
    }
  }
  //        event->ignore();
  QGraphicsView::mouseMoveEvent(event);
}

void gQGraphicsView::mouseReleaseEvent(QMouseEvent *event) {
  if (_pan)
    if (event->button() == Qt::LeftButton) {
      _pan = false;
      setCursor(Qt::ArrowCursor);
      event->accept();
      return;
    }
  //    event->ignore();
  QGraphicsView::mouseReleaseEvent(event);
}

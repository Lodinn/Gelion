#include "view.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

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
    QGraphicsView::mousePressEvent(event);
    return;
  }

  if (event->modifiers() == Qt::ControlModifier)
    if (event->button() == Qt::LeftButton) {
      QPointF point = mapToScene(event->pos());
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
  if (scene()->mouseGrabberItem() != nullptr) {
    qDebug() << mapToScene(event->pos()) << scene()->mouseGrabberItem() << ROIs;
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

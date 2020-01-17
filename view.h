#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QPixmap>

class ROI : public QObject, public QGraphicsItem {
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)
 public:
  enum shape_t { Point, Line, Circle, Rectangle, Polygon } shape;
  explicit ROI(QPointF position, QString label = QString(), shape_t t = Point);
  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget);
  void update_position(QPointF new_pos) { pos = new_pos; }
  void update_label(QString new_label) { label = new_label; }

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

 private:
  QPointF pos;
  QString label;
};

class gQGraphicsView : public QGraphicsView {
  Q_OBJECT

 public:
  explicit gQGraphicsView(QGraphicsScene *scene = nullptr);
 public slots:

 protected:
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

 private:
  bool _pan;
  int _panX0, _panY0;
  QList<QGraphicsItem *> ROIs;
  QList<ROI *> ROIs_new;
 signals:
  void point_picked(QPointF);
  // FIXME: keeping two separate index lists here and in mainwindow. Subclass
  // instead and connect to plot directly?
  void roi_position_updated(QPair<int, QPointF> new_position);
};

#endif  // VIEW_H

#ifndef VIEW_H
#define VIEW_H

#include "ImageHandler.h"
#include "zgraph.h"

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QPixmap>

class ROI;

class gQGraphicsView : public QGraphicsView {
  Q_OBJECT

public:
  explicit gQGraphicsView(QGraphicsScene *scene = nullptr);
    enum insertItem { None, Point, Rect, Ellipse, Polyline, Polygon };
    insertItem insertMode = insertItem::None;
    ImageHandler *imgHand;
public slots:
    void setInputModePoint();
    void setInputModeRect();
    void setInputModeEllipse();
    void setInputModePolyline();
    void setInputModePolygon();
    void selectionZChanged();
    void show_profile_for_Z(zGraph *item);
    void moveGrabberRects();
    void releaseGrabberRects();
    void pressOneGrabberRect();
    void releaseOneGrabberRect();
    void moveOneGrabberRect(int num, QPointF point);
public:
    qreal GlobalScale = 1;  qreal GlobalRotate = 0;  int GlobalChannelNum = 0;
    QAction *openAct;
    QAction *pointAct;
    QAction *rectAct;
    QAction *ellipseAct;
    QAction *polylineAct;
    QAction *polygonAct;
    QAction *closeAct;
    QAction *winZGraphListAct;
    QAction *channelListAct;
    QAction *winZGraphListShowAllAct;
    QAction *winZGraphListHideAllAct;
    zGraph *grabberItem = 0;
    QList<zContourRect *> zcRects;
    zRect *tmpRect = nullptr;
    zEllipse *tmpEllipse = nullptr;
    QList<QGraphicsLineItem *> tmpLines;
    QList<zGraph *> getZGraphItemsList();
    void clearZGraphItemsList();
 protected:
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;
  void insPoint(QPoint pos);
  void deleteGrabberRects();
 private:
  bool PAN;
  int pX0, pY0;
  QList<QGraphicsItem *> ROIs;
  QList<ROI *> ROIs_new;
  void setScaleItems(qreal &newscale);
  void setRotateItems(qreal &newangle);
  void createRect();
  void createEllipse();
  void setGrabberCoordTozRect();
  void setGrabberCoordTozEllipse();
  void createPolygon();
  void createPolyline();
  void multiPointsReplotRect(zGraph *item);
  void multiPointsReplotPoly(zGraph *item);
  void setCursorToItems();

  QPen fInsPen = QPen(Qt::red, 2, Qt::DashDotLine);
  bool contextMenuEnable = true;
 signals:
  void insertZGraphItem(zGraph *item);
  void setZGraphDockToggled(zGraph *item);

  void point_picked(QPointF);
  // FIXME: keeping two separate index lists here and in mainwindow. Subclass
  // instead and connect to plot directly?
  void roi_position_updated(QPair<int, QPointF> new_position);
};

class ROI : public QObject, public QGraphicsItem {
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)
 public:
  enum shape_t { Point, Line, Circle, Rectangle, Polygon } shape;
  explicit ROI(QPointF position, QString label = QString(), shape_t t = Point);
  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;
  void update_position(QPointF new_pos) {
    pos = new_pos;
    update();
  }
  void update_label(QString new_label) { label = new_label; }

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  //  signals:
  //    void moved();

 private:
  QPointF pos;
  QString label;
};

#endif  // VIEW_H

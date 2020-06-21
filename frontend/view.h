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
    void moveGrabberRects();
    void releaseGrabberRects();
    void pressOneGrabberRect();
    void releaseOneGrabberRect();
    void moveOneGrabberRect(int num, QPointF point);
    void selectionZChanged();
    void deleteZGraphItem(zGraph *item);
    void show_profile_for_Z(zGraph *item);
public:
    qreal GlobalScale = 1;  qreal GlobalRotate = 0;
    int GlobalChannelNum = 0;  int GlobalChannelStep = 1;
    bool empty = true;  // сцена не содержит данных
    QList<zGraph *> getZGraphItemsList();  // список рафических объектов на сцене
    QAction *openAct;
    QAction *localFolderAct = new QAction(QIcon(":/icons/folder.png"), "Текущий каталог ...");
    QAction *debugFolderAct = new QAction(QIcon(":/icons/delete-32.png"), "debug ... очистить текущий каталог ...");
    QAction *pointAct;
    QAction *rectAct;
    QAction *ellipseAct;
    QAction *polylineAct;
    QAction *polygonAct;
    QAction *closeAct;
    QAction *winZGraphListAct;
    QAction *indexListAct;
    QAction *channelListAct;
    QAction *maskListAct;
    QAction *winZGraphListShowAllAct;
    QAction *winZGraphListHideAllAct;
    zGraph *grabberItem = nullptr;
    zRect *tmpRect = nullptr;
    zEllipse *tmpEllipse = nullptr;
    QList<zContourRect *> zcRects;
    QVector<QGraphicsLineItem *> tmpLines;
    void clearZGraphItemsList();
    void deleteGrabberRects();
    void deleteTmpLines();
    void clearForAllObjects();
    bool PAN = false;
    QGraphicsPixmapItem *mainPixmap;
    QPixmap changeBrightnessPixmap(QImage &img, qreal brightness);
    QString index_title_str, index_formula_str;
    QColor waveLengthToRGB(double Wavelength);  // 380 - 781
 protected:
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;
 private:
  int pX0, pY0;
  QList<QGraphicsItem *> ROIs;
  QList<ROI *> ROIs_new;
  void setScaleItems(qreal &newscale);
  void setRotateItems(qreal &newangle);
  void setGrabberCoordTozRect();
  void setGrabberCoordTozEllipse();
  void insPoint(QPoint pos);
  void createPolygon(QPoint pos);
  void createPolyline(QPoint pos);
  void multiPointsReplotRect(zGraph *item);
  void multiPointsReplotPoly(zGraph *item);
  QPen fInsPen = QPen(Qt::red, 2, Qt::DashDotLine);
  bool contextMenuEnable = true;
 signals:
  void insertZGraphItem(zGraph *item);  // main createDockWidgetForItem
  void setZGraphDockToggled(zGraph *item);  // main toggleViewAction
  void point_picked(QPointF);
  void removeFromzGraphListWidget(zGraph *item);
  void changeZObject(zGraph *item);

// FIXME: keeping two separate index lists here and in mainwindow. Subclass
// instead and connect to plot directly?
//  void roi_position_updated(QPair<int, QPointF> new_position);
};

/*class ROI : public QObject, public QGraphicsItem {
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
}; */

#endif  // VIEW_H

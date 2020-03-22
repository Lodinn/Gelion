#ifndef ZVIEW_H
#define ZVIEW_H

#include "../zgraph.h"

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QAction>
#include <QGraphicsProxyWidget>

// Расширяем класс QGraphicsView
class zView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit zView(QWidget *parent = 0);
    ~zView();

    enum insertItem { None, Point, Rect, Ellipse, Polyline, Polygon };
    insertItem insertMode = insertItem::None;
    QAction *dockAct;
    QAction *closeAct;
    QAction *docksaveAct;
    QAction *dockrestoreAct;

    QAction *setSelAct;
    QAction *visibleDock;

    //----------------------------------------------------- main
    zGraph *grabberItem = 0;
    QList<zContourRect *> zcRects;
    QAction *pointAct;
    QAction *polygonAct;
    QAction *rectAct;
    QList<QGraphicsLineItem *> tmpLines;
    zRect *tmpRect = nullptr;
    QGraphicsProxyWidget *w = nullptr;
    //----------------------------------------------------- main

public slots:
    void testSlot();
    void itemChange();
    void setSelectionForce();
    //----------------------------------------------------- main
    void createGrabberRects();
    void deleteGrabberRects();
    void moveGrabberRects();
    void pressOneGrabberRect();
    void moveOneGrabberRect(int num, QPointF point);
    void releaseOneGrabberRect();
    void setInputModePoint();
    void setInputModePolygon();
    void setInputModeRect();
protected:
    QPointF contextPos;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void createPointGraphItem(QPoint pos);
    void wheelEvent(QWheelEvent *event) override;
private:
    int num = 0;
    QPen fInsPen = QPen(Qt::red, 2, Qt::DashDotLine);
    QAction *testAct;
    qreal zScale = 1.0;  qreal zAngle = 0.0;
    void setScaleAngle(qreal sc, qreal an);
    bool contextMenuEnable = true;
    void createPolygon();
    void createRect();
    void showMessageWidget(QString str);
    void setGrabberCoordTozRect();
    qreal GlobalScale = 1.0;
    qreal GlobalRotate = 0.0;
    //----------------------------------------------------- main
signals:
    void updateItemsList();
};

#endif // ZVIEW_H

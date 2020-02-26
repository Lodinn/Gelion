#ifndef VIEW_H
#define VIEW_H

#include <QAction>
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include <QIcon>
#include <QPixmap>

class wview : public QGraphicsView
{
    Q_OBJECT

public:
    explicit wview(QGraphicsScene *scene = 0);
    enum insertItem { None, Point, Ellipse, Rect, Polyline, Polygon };
    insertItem insertMode = insertItem::None;
    QAction *closeAct;
    QAction *testAct;
public slots:
    void insEllipseItemSLOT();
    void insPolylineItemSLOT();
    void testSLOT();
protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    int contextMenuTrigger = 0;
    void addwEllipseToScene();
    void addwPolylineToScene();

private:
    bool _pan;
    int _panX0, _panY0;
    QGraphicsLineItem *tmpLine = nullptr;
    QAction *insPointItemAct;
    QAction *insEllipseItemAct;
    QAction *insRectItemAct;
    QAction *insPolylineItemAct;
    QAction *insPolygonItemAct;
    QGraphicsEllipseItem *tmpEllipse = nullptr;
    QList<QGraphicsLineItem *> tmpLineItems;
    QList<QPointF> tmpPoints;

    void createActions()
    {
        const QIcon insPointIcon = QIcon("../second/images/pin2_red.png");
        const QIcon insEllipseIcon = QIcon("../second/images/vector_ellipse.png");
        const QIcon insRectIcon = QIcon("../second/images/vector_square.png");
        const QIcon insPolylineIcon = QIcon("../second/images/polyline-64.png");
        const QIcon insPolygonIcon = QIcon("../second/images/vector-polygon.png");
        QString str("Добавить область интереса - %1");
        insPointItemAct = new QAction(insPointIcon, str.arg("точка (Ctrl + mouse left)"), this);
        insEllipseItemAct = new QAction(insEllipseIcon, str.arg("эллипс"), this);
        insRectItemAct = new QAction(insRectIcon, str.arg("прямоугольник"), this);
        insPolylineItemAct = new QAction(insPolylineIcon, str.arg("полилиния"), this);
        insPolygonItemAct = new QAction(insPolygonIcon, str.arg("область(полигон)"), this);
        closeAct = new QAction(insPointIcon, "exit", this);
        testAct = new QAction(insPointIcon, "test", this);
    }
signals:

};

#endif // VIEW_H

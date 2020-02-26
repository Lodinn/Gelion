#ifndef ZVIEW_H
#define ZVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QAction>

// Расширяем класс QGraphicsView
class zView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit zView(QWidget *parent = 0);
    ~zView();

    enum insertItem { None, Point, Ellipse, Rect, Polyline, Polygon };
    insertItem insertMode = insertItem::None;
    QAction *dockAct;
    QAction *closeAct;
    QAction *docksaveAct;
    QAction *dockrestoreAct;
public slots:
    void testSlot();
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
private:
    QAction *testAct;
signals:

};

#endif // ZVIEW_H

#ifndef MSCENE_H
#define MSCENE_H

#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class mscene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum insertItem { None, Point, Polyline, Region };

    explicit mscene(QObject *parent = 0);
    ~mscene();

    insertItem insertMode;
    QGraphicsLineItem *tmpLineItem;

private:


public slots:

protected:
      void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
      void mousePressEvent(QGraphicsSceneMouseEvent *event) override;


};

#endif // MSCENE_H

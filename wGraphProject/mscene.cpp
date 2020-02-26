#include "mscene.h"

#include <QCursor>
#include <QDebug>
#include <QMenu>

mscene::mscene(QObject *parent) :  QGraphicsScene(parent)
{
    this->insertMode = mscene::None;
    this->setFocus();
}

mscene::~mscene()
{

}

void mscene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)

{
    qDebug() << "mscene::mouseMoveEvent" << event->scenePos();
    QGraphicsScene::mouseMoveEvent(event);
//    switch (insertMode) {
//    case mscene::None : {
//        QGraphicsScene::mouseMoveEvent(event);
//        qDebug() << "mscene::None" << event->scenePos() << insertMode;
//        return;
//    }
//    case mscene::Region : {
//        tmpLineItem->line().setP2(event->scenePos());
//        tmpLineItem->update();

//        QGraphicsScene::mouseMoveEvent(event);
//        qDebug() << "mscene::Region" << event->scenePos() << insertMode;
//        return;
//    }
//    default:
//        break;
//    }
}

void mscene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
//    this->insertMode = mscene::Region;
//    qDebug() << "insertMode" << insertMode;
//    switch (insertMode) {
//    case mscene::None : {
//        QGraphicsScene::mousePressEvent(event);
//        return;
//    }
//    case mscene::Region : {
//        QLineF line(event->scenePos(), event->scenePos());
//        tmpLineItem = addLine(line, QPen(Qt::red, 7, Qt::DashLine));
//        tmpLineItem->setVisible(true);
//        tmpLineItem->setFlag(QGraphicsItem::ItemIsMovable, false);
//        tmpLineItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
//        QGraphicsScene::mousePressEvent(event);
//        return;
//    }
//    default:
//        break;
//    }
}

#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class wscene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit wscene(QObject *parent = 0);
    ~wscene();

private:

public slots:

protected:

};

#endif // SCENE_H

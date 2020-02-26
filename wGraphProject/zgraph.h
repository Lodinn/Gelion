#ifndef ZGRAPH_H
#define ZGRAPH_H

#include <QGraphicsItem>

class zGraph : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit zGraph();
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual void updateBoundingRect();
    void setfRect();
    void setTitle(QString title);
    QString getTitle();
    virtual QPointF getCenter();
signals:
     void mouseMove();
private:
    QRectF fbrect;
};

#endif // ZGRAPH_H

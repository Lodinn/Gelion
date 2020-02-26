#include "zgraph.h"

zGraph::zGraph()
{

}

QRectF zGraph::boundingRect() const
{
    return fbrect;
}

void zGraph::updateBoundingRect()
{

}

void zGraph::setTitle(QString title)
{
    setData(0, title);
}

QString zGraph::getTitle()
{
    return data(0).toString();
}

QPointF zGraph::getCenter()
{

}

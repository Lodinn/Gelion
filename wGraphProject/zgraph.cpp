#include "zgraph.h"

#include <QPainter>
#include <QDebug>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>

zGraph::zGraph()
{
    setData(1,1.0);   setData(2,0.0);
}

QRectF zGraph::boundingRect() const
{
    return fbrect;
}

void zGraph::setTitle(QString title)
{
    setData(0, title);
}

QString zGraph::getTitle() const
{
    return data(0).toString();
}

void zGraph::setFont(QString family, int pointSize, int weight, bool italic)
{
    ffont = QFont(family, pointSize, weight, italic);
}

void zGraph::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    emit mouseMove();
    QGraphicsObject::mouseMoveEvent(event);
}

zPoint::zPoint(const QPointF point) :  zGraph()
{
    setPos(point);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}

QRectF zPoint::getTitleRectangle() const
{
    QString title = getTitle();
    title = QString(" %1 ").arg(title);
    QFontMetricsF fm(ffont);
    int tw = fm.width(title);
    int th = fm.height();
    return QRectF(0.5*fRectIndent,-0.5*fRectIndent-th,tw,th);
}

QPainterPath zPoint::shape() const
{
    QPainterPath path;
    path.addEllipse(-fRectIndent/2,-fRectIndent/2,fRectIndent,fRectIndent);
    path.addRect(ftitleRect);
    return path;
}

void zPoint::updateBoundingRect()
{
    ftitleRect = getTitleRectangle();
    qreal left = -fRectIndent;  qreal top = -fRectIndent - 2*ftitleRect.height();
    qreal width = 3*fRectIndent + ftitleRect.width();
    qreal height = 2*ftitleRect.height() + 2*fRectIndent;
    fbrect = QRectF(left,top,width,height);
}

void zPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (isSelected()) {
        painter->setPen(selectedPen);
        painter->setBrush(selectedBrush);
    } else {
        painter->setPen(basePen);
        painter->setBrush(baseBrush);
    }
//    painter->drawRect(fbrect);
    painter->drawEllipse(-fRectIndent/2,-fRectIndent/2,fRectIndent,fRectIndent);
    painter->drawLine(QPointF(0,-fRectIndent/2),QPointF(0,fRectIndent/2));
    painter->drawLine(QPointF(-fRectIndent/2,0),QPointF(fRectIndent/2,0));
// наименование объекта
    setOpacity(fOpacity);
//    painter->drawLine(fRectIndent,-30,fRectIndent,30);
//    painter->drawLine(-30,-fRectIndent,30,-fRectIndent);
    QString title = getTitle();
    painter->setFont(ffont);
    painter->setPen(titleRectPen);
    painter->setBrush(titleRectBrush);
    painter->drawRoundRect(ftitleRect,12,25);
    painter->setPen(titleTextPen);
    painter->drawText(ftitleRect,QString(" %1 ").arg(title));
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

zPolygon::zPolygon() :  zGraph()
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setData(1,1.0);   setData(2,0.0);
}

QRectF zPolygon::getTitleRectangle() const
{
    QString title = getTitle();
    title = QString(" %1 ").arg(title);
    QFontMetricsF fm(ffont);
    int tw = fm.width(title);
    int th = fm.height();
    return QRectF(fcenterPoint.x()-tw/2.0,fcenterPoint.y()+th/2.0,tw,th);
}

QPainterPath zPolygon::shape() const
{
    QPainterPath path;
    path.addPolygon(fpolygon);
    path.addRect(ftitleRect);
    return path;
}

void zPolygon::updateBoundingRect()
{
    fcenterPoint = getCenterPoint();
    ftitleRect = getTitleRectangle();
    QPainterPath path;
    path.addPolygon(fpolygon);
    path.addRect(ftitleRect);
    fbrect = path.boundingRect();
}

void zPolygon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (isSelected()) {
        painter->setPen(selectedPen);
        painter->setBrush(selectedBrush);
    } else {
        painter->setPen(basePen);
        painter->setBrush(baseBrush);
    }
    painter->drawPolygon(fpolygon);
    // наименование объекта
    setOpacity(fOpacity);
    QString title = getTitle();
    painter->setFont(ffont);    painter->setPen(titleRectPen);    painter->setBrush(titleRectBrush);
    painter->save();
    painter->scale(data(1).toDouble(),data(1).toDouble());
    painter->rotate(data(2).toDouble());
    painter->drawRoundRect(ftitleRect,12,25);
    painter->setPen(titleTextPen);
    painter->drawText(ftitleRect,QString(" %1 ").arg(title));
    painter->restore();
    painter->drawLine(fcenterPoint.x(),fcenterPoint.y()-100,fcenterPoint.x(),fcenterPoint.y()+100);
    painter->drawLine(fcenterPoint.x()-100,fcenterPoint.y(),fcenterPoint.x()+100,fcenterPoint.y());
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

void zPolygon::addPoint(QPoint point)
{
    fpolygon << point;
}

QPointF zPolygon::getCenterPoint()
{
    if (fpolygon.isEmpty()) return QPointF(0,0);
    QPointF point;
    qreal x = 0.0;  qreal y = 0.0;
    foreach ( QPoint p , fpolygon ) {
        x += p.rx();  y += p.ry();
    }
    point.setX(x/fpolygon.length());
    point.setY(y/fpolygon.length());
    return point;
}

void zPolygon::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QMessageBox msgBox;
    msgBox.setText("The document has been modified.");
    msgBox.exec();
}

zContourRect::zContourRect(QPointF point)
{
    setObjectName("zContourRect");
    setBrush(fBrush);   setPen(fPen);
    setRect(-fRectIndent,-fRectIndent,2*fRectIndent,2*fRectIndent);
    setPos(point);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setCursor(Qt::CrossCursor);
}

zContourRect::~zContourRect()
{

}

void zContourRect::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit mousePress();
    QGraphicsRectItem::mousePressEvent(event);
}

void zContourRect::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    emit mouseMove(data(0).toInt(), event->scenePos());
    QGraphicsRectItem::mouseMoveEvent(event);
}

void zContourRect::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit mouseRelease();
    QGraphicsRectItem::mouseReleaseEvent(event);
}

zRect::zRect(const QPointF point)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    frectSize = QSizeF(0,fdefaultHeigth);
    setPos(point);
}

QRectF zRect::getTitleRectangle() const
{
    QString title = getTitle();
    title = QString(" %1 ").arg(title);
    QFontMetricsF fm(ffont);
    int tw = fm.width(title);
    int th = fm.height();
    return QRectF(fcenterPoint-QPointF(tw/2,th/2),QSizeF(tw,th));
}

QPainterPath zRect::shape() const
{
    QPainterPath path;
    qreal w = frectSize.width();  qreal h = frectSize.height();
    path.addRect(0,-h/2,w,h);
    path.addRect(ftitleRect);
    return path;
}

void zRect::updateBoundingRect()
{
    fcenterPoint = getCenterPoint();
    ftitleRect = getTitleRectangle();
    QPainterPath path;
    qreal w = frectSize.width();  qreal h = frectSize.height();
    path.addRect(0,-h/2,w,h);
    path.addRect(ftitleRect);
    fbrect = path.boundingRect();
}

void zRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (isSelected()) {
        painter->setPen(selectedPen);
        painter->setBrush(selectedBrush);
    } else {
        painter->setPen(basePen);
        painter->setBrush(baseBrush);
    }
    qreal w = frectSize.width();  qreal h = frectSize.height();
    painter->drawRect(0,-h/2,w,h);
    if (isSelected()) {
        painter->drawLine(0,0,w,0);
        painter->drawLine(w/2,-h/2,w/2,h/2);
    }
    // наименование объекта
    setOpacity(fOpacity);
    QString title = getTitle();
    painter->setFont(ffont);    painter->setPen(titleRectPen);    painter->setBrush(titleRectBrush);
    painter->save();
    painter->scale(data(1).toDouble(),data(1).toDouble());
    painter->rotate(data(2).toDouble());
    painter->drawRoundRect(ftitleRect,12,25);
    painter->setPen(titleTextPen);
    painter->drawText(ftitleRect,QString(" %1 ").arg(title));
    painter->restore();
    painter->drawLine(fcenterPoint.x(),fcenterPoint.y()-100,fcenterPoint.x(),fcenterPoint.y()+100);
    painter->drawLine(fcenterPoint.x()-100,fcenterPoint.y(),fcenterPoint.x()+100,fcenterPoint.y());
    setOpacity(fOpacity);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

QPointF zRect::getCenterPoint()
{
    qreal w = frectSize.width();
    return QPointF(w/2,0);
}

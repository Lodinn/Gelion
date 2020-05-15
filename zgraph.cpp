#include "zgraph.h"

#include <QPainter>
#include <QDebug>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QtMath>

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

void zGraph::setParamsToDialog(zgraphParamDlg *dlg)
{
    dlg->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
    dlg->ui->lineEditTitle->setText(this->getTitle());  // наименование
    dlg->ui->labelTypeTitle->setText(this->typeString);  // тип объекта
    dlg->ui->checkBoxWdock->setChecked(this->dockw->isVisible());  // видимость графика профиля
    dlg->ui->checkBoxZGraphVisible->setChecked(this->isVisible());  // видимость объекта
}

void zGraph::getParamsFromDialog(zgraphParamDlg *dlg)
{
    QString str = dlg->ui->lineEditTitle->text();
    this->setTitle(str);
    this->setVisible(dlg->ui->checkBoxZGraphVisible->isChecked());
    this->dockw->setWindowTitle(str);
    this->dockw->setVisible(dlg->ui->checkBoxWdock->isChecked());
    if (this->isVisible()) listwidget->setCheckState(Qt::Checked);
    else listwidget->setCheckState(Qt::Unchecked);
    QFont font = listwidget->font();  font.setBold(this->dockw->isVisible());
    listwidget->setFont(font);
    listwidget->setText(this->getTitle());
    this->updateBoundingRect();
}

QPointF zGraph::getRectPoint2(QPointF p1, qreal w, qreal rot)
{
    QPointF point;
    double xpos = p1.rx() + w * qCos(qDegreesToRadians(rot));
    double ypos = p1.ry() + w * qSin(qDegreesToRadians(rot));
    point.setX(xpos + pos().rx());  point.setY(ypos + pos().ry());
    return point;
}

double zGraph::getRotationFromCoords(QPointF p1, QPointF p2)
{
    qreal dx  = p2.x() - p1.x();
    qreal dy  = p2.y() - p1.y();
    return qRadiansToDegrees( qAtan2(dy, dx) );
}

void zGraph::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    emit mouseMove();
    QGraphicsObject::mouseMoveEvent(event);
}

void zGraph::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit mouseRelease();
    QGraphicsObject::mouseReleaseEvent(event);
}

QVariant zGraph::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    emit itemChange();
    return QGraphicsObject::itemChange(change, value);
}

void zGraph::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
//    qDebug() << "zGraph::mouseDoubleClickEvent";
    zgraphParamDlg *dlg = new zgraphParamDlg();
    dlg->move(event->screenPos());
    setParamsToDialog(dlg);
    if (dlg->exec() == QDialog::Accepted) {
        if (dlg->ui->checkBoxZGraphDelete->isChecked()) {
            emit itemDelete(this);
            return;
        }  // if
        getParamsFromDialog(dlg);
    }  // if
    //    QGraphicsObject::mouseDoubleClickEvent(event);
}

zPoint::zPoint(const QPointF point) :  zGraph()
{
    setPos(point);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    typeString = "Точка";
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

bool zPoint::pointIn(QPointF point)
{
    Q_UNUSED(point)
    return true;
}

QPolygonF zPoint::getTitlePolygon()
{
    return QPolygonF();
}

QStringList zPoint::getSettings(int num)
{
    QStringList strlist;
    // rectangle7/title#Прямоугольник 1
    strlist.append(QString("Point%1/objectvisible%2%3").arg(num).arg(setsep).arg(this->isVisible()));
    strlist.append(QString("Point%1/title%2%3").arg(num).arg(setsep).arg(getTitle()));
    strlist.append(QString("Point%1/pos%2%3,%4").arg(num).arg(setsep)
                   .arg(pos().rx(),0,'f',2).arg(pos().ry(),0,'f',2));
    strlist.append(QString("Point%1/dockvisible%2%3").arg(num).arg(setsep).arg(dockw->isVisible()));
    QPoint dwpos = dockw->geometry().topLeft();
    strlist.append(QString("Point%1/dockwpos%2%3,%4").arg(num).arg(setsep)
                   .arg(dwpos.x()).arg(dwpos.y()));
    return strlist;
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
    QString title = getTitle();
    painter->setFont(ffont);
    painter->setPen(titleRectPen);
    if (isSelected()) painter->setBrush(titleRectBrushSelection);
    else painter->setBrush(titleRectBrush);
    painter->drawRoundRect(ftitleRect,12,25);
    painter->setPen(titleTextPen);
    painter->drawText(ftitleRect,QString(" %1 ").arg(title));
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

zPolygon::zPolygon(qreal scale, qreal rotate) :  zGraph()
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setData(1,scale);  setData(2,rotate);
    typeString = "Полигон";
}

QRectF zPolygon::getTitleRectangle() const
{
    QString title = getTitle();
    title = QString(" %1 ").arg(title);
    QFontMetricsF fm(ffont);
    int tw = fm.width(title);
    int th = fm.height();
    return QRectF(fcenterPoint-QPointF(tw/2,th/2),QSizeF(tw,th));
}

QPainterPath zPolygon::shape() const
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill); // то что надо
    path.addPolygon(fpolygon);
    path.addPolygon(ftitlePolygon);
    return path;
}

void zPolygon::updateBoundingRect()
{
    fcenterPoint = getCenterPoint();
    ftitleRect = getTitleRectangle();
    ftitlePolygon = getTitlePolygon();
    QPainterPath path;
    path.setFillRule(Qt::WindingFill); // то что надо
    path.addPolygon(fpolygon);
    path.addPolygon(ftitlePolygon);
    fbrect = path.boundingRect();
}

bool zPolygon::pointIn(QPointF point)
{
    return fpolygon.containsPoint(point.toPoint(),Qt::OddEvenFill);
}

QPolygonF zPolygon::getTitlePolygon()
{
    QMatrix matrix;
    QPolygonF polygon;

    QPointF pcenter = getCenterPoint() / data(1).toDouble();
    QLineF line(QPointF(0,0),pcenter);
    qreal acenter = qAtan2(line.dy(),line.dx());
    qreal R = qDegreesToRadians(-data(2).toDouble());
    R += acenter;
    pcenter.setX(line.length()*qCos(R));
    pcenter.setY(line.length()*qSin(R));

    QString title = getTitle();
    title = QString(" %1 ").arg(title);
    QFontMetricsF fm(ffont);
    int tw = fm.width(title);
    int th = fm.height();
    QRect rect = QRect(pcenter.toPoint()-QPoint(tw/2,th/2),QSize(tw,th));

    polygon = matrix.mapToPolygon(rect);
    matrix.scale(data(1).toDouble(),data(1).toDouble());
    matrix.rotate(data(2).toDouble()-rotation());

    return matrix.map(polygon);
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
    painter->drawPolygon(ftitlePolygon);
    // наименование объекта
    setOpacity(fOpacity);
    QString title = getTitle();
    painter->setFont(ffont);    painter->setPen(titleRectPen);    painter->setBrush(titleRectBrush);
    fcenterPoint = getCenterPoint();
    painter->save();
    painter->scale(data(1).toDouble(),data(1).toDouble());  // ===
    painter->rotate(data(2).toDouble()-rotation());
    fcenterPoint = getCenterPoint() / data(1).toDouble();
    QLineF line(QPointF(0,0),fcenterPoint);
    qreal centerAngle = qAtan2(line.dy(),line.dx());
    qreal R = qDegreesToRadians(-data(2).toDouble());
    R += centerAngle;
    fcenterPoint.setX(line.length()*qCos(R));
    fcenterPoint.setY(line.length()*qSin(R));
    ftitleRect = getTitleRectangle();
    if (isSelected()) painter->setBrush(titleRectBrushSelection);
    else painter->setBrush(titleRectBrush);
    painter->drawRoundRect(ftitleRect,12,25);
    painter->setPen(titleTextPen);
    painter->drawText(ftitleRect,QString(" %1 ").arg(title));
    painter->restore();                                     // ===
    fcenterPoint = getCenterPoint();
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

void zPolygon::addPoint(QPoint point)
{
    fpolygon << point;
}

QStringList zPolygon::getSettings(int num)
{
    QStringList strlist;
    // polygon7/title#Полигон 1
    strlist.append(QString("Polygon%1/objectvisible%2%3").arg(num).arg(setsep).arg(this->isVisible()));
    strlist.append(QString("Polygon%1/title%2%3").arg(num).arg(setsep).arg(getTitle()));
    strlist.append(QString("Polygon%1/pos%2%3,%4").arg(num).arg(setsep)
                   .arg(pos().rx(),0,'f',2).arg(pos().ry(),0,'f',2));
    QString xstr, ystr;
    foreach(QPoint point, fpolygon) {
      xstr.append(QString::number(point.rx(), 'f', 2));
      ystr.append(QString::number(point.ry(), 'f', 2));
      if (fpolygon.indexOf(point) < fpolygon.length()-1) {
        xstr.append(",");
        ystr.append(",");
      }
    }
    strlist.append(QString("Polygon%1/x%2%3").arg(num).arg(setsep).arg(xstr));
    strlist.append(QString("Polygon%1/y%2%3").arg(num).arg(setsep).arg(ystr));
    strlist.append(QString("Polygon%1/dockvisible%2%3").arg(num).arg(setsep).arg(dockw->isVisible()));
    QPoint dwpos = dockw->geometry().topLeft();
    strlist.append(QString("Polygon%1/dockwpos%2%3,%4").arg(num).arg(setsep)
                   .arg(dwpos.x()).arg(dwpos.y()));
    return strlist;
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

zRect::zRect(const QPointF point, qreal scale, qreal rotate)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    frectSize = QSizeF(0,fdefaultHeigth);
    setPos(point);
    setData(1,scale);  setData(2,rotate);
    typeString = "Прямоугольник";
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
    path.addPolygon(ftitlePolygon);
    return path;
}

void zRect::updateBoundingRect()
{
    fcenterPoint = getCenterPoint();
    ftitleRect = getTitleRectangle();
    ftitlePolygon = getTitlePolygon();
    QPainterPath path;
    qreal w = frectSize.width();  qreal h = frectSize.height();
    path.addRect(0,-h/2,w,h);
    path.addPolygon(ftitlePolygon);
    fbrect = path.boundingRect();
}

bool zRect::pointIn(QPointF point)
{
    QPainterPath path;
    qreal w = frectSize.width();  qreal h = frectSize.height();
    path.addRect(0,-h/2,w,h);
    return path.contains(point);
}

QPolygonF zRect::getTitlePolygon()
{
    QMatrix matrix;
    QPolygonF polygon;

    QPointF pcenter = getCenterPoint() / data(1).toDouble();
    QLineF line(QPointF(0,0),pcenter);
    qreal acenter = qAtan2(line.dy(),line.dx());
    qreal R = qDegreesToRadians(rotation()-data(2).toDouble());
    R += acenter;
    pcenter.setX(line.length()*qCos(R));
    pcenter.setY(line.length()*qSin(R));

    QString title = getTitle();
    title = QString(" %1 ").arg(title);
    QFontMetricsF fm(ffont);
    int tw = fm.width(title);
    int th = fm.height();
    QRect rect = QRect(pcenter.toPoint()-QPoint(tw/2,th/2),QSize(tw,th));

    polygon = matrix.mapToPolygon(rect);
    matrix.scale(data(1).toDouble(),data(1).toDouble());
    matrix.rotate(data(2).toDouble()-rotation());

    return matrix.map(polygon);
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
//    painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
//    painter->setCompositionMode(QPainter::CompositionMode_Xor);
    painter->drawRect(0,-h/2,w,h);
    painter->drawPolygon(ftitlePolygon);
    if (isSelected()) {
        painter->drawLine(0,0,w,0);
        painter->drawLine(w/2,-h/2,w/2,h/2);
    }
// наименование объекта
    setOpacity(fOpacity);
    QString title = getTitle();
    painter->setFont(ffont);    painter->setPen(titleRectPen);    painter->setBrush(titleRectBrush);
    painter->save();
    painter->scale(data(1).toDouble(),data(1).toDouble());  // ===
    painter->rotate(data(2).toDouble()-rotation());
    fcenterPoint = getCenterPoint() / data(1).toDouble();
    QLineF line(QPointF(0,0),fcenterPoint);
    qreal R = qDegreesToRadians(rotation()-data(2).toDouble());
    fcenterPoint.setX(line.length()*qCos(R));
    fcenterPoint.setY(line.length()*qSin(R));
    ftitleRect = getTitleRectangle();
    if (isSelected()) painter->setBrush(titleRectBrushSelection);
    else painter->setBrush(titleRectBrush);
    painter->drawRoundRect(ftitleRect,12,25);
    painter->setPen(titleTextPen);
    painter->drawText(ftitleRect,QString(" %1 ").arg(title));
    painter->restore();                                     // ===
    fcenterPoint = getCenterPoint();
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

QStringList zRect::getSettings(int num)
{
    QStringList strlist;
    // rectangle7/title#Прямоугольник 1
    strlist.append(QString("Rectangle%1/objectvisible%2%3").arg(num).arg(setsep).arg(this->isVisible()));
    strlist.append(QString("Rectangle%1/title%2%3").arg(num).arg(setsep).arg(getTitle()));
    QPointF point = getRectPoint2(pos(), frectSize.width(), rotation());
    strlist.append(QString("Rectangle%1/pos%2%3,%4").arg(num).arg(setsep)
                   .arg(pos().rx(),0,'f',2).arg(pos().ry(),0,'f',2)
                   /*.arg(point.rx(),0,'f',2).arg(point.ry(),0,'f',2)*/);
    qreal angle = rotation();
    strlist.append(QString("Rectangle%1/rotation%2%3").arg(num).arg(setsep).arg(angle,0,'f',2));
    strlist.append(QString("Rectangle%1/size%2%3,%4").arg(num).arg(setsep)
                   .arg(frectSize.width(),0,'f',2).arg(frectSize.height(),0,'f',2));
    strlist.append(QString("Rectangle%1/dockvisible%2%3").arg(num).arg(setsep).arg(dockw->isVisible()));
    QPoint dwpos = dockw->geometry().topLeft();
    strlist.append(QString("Rectangle%1/dockwpos%2%3,%4").arg(num).arg(setsep)
                   .arg(dwpos.x()).arg(dwpos.y()));
    return strlist;
}

QPointF zRect::getCenterPoint()
{
    qreal w = frectSize.width();
    QPointF cp = QPointF(w/2,0);
    return cp;
}

zEllipse::zEllipse(const QPointF point, qreal scale, qreal rotate)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    frectSize = QSizeF(0,fdefaultHeigth);
    setPos(point);
    setData(1,scale);  setData(2,rotate);
    typeString = "Эллипс";
}

QRectF zEllipse::getTitleRectangle() const
{
    QString title = getTitle();
    title = QString(" %1 ").arg(title);
    QFontMetricsF fm(ffont);
    int tw = fm.width(title);
    int th = fm.height();
    return QRectF(fcenterPoint-QPointF(tw/2,th/2),QSizeF(tw,th));
}

QPainterPath zEllipse::shape() const
{
    QPainterPath path;
    qreal w = frectSize.width();  qreal h = frectSize.height();
    path.addEllipse(0,-h/2,w,h);
    path.addPolygon(ftitlePolygon);
    return path;
}

void zEllipse::updateBoundingRect()
{
    fcenterPoint = getCenterPoint();
    ftitleRect = getTitleRectangle();
    ftitlePolygon = getTitlePolygon();
    QPainterPath path;
    path.setFillRule(Qt::WindingFill); // то что надо
    qreal w = frectSize.width();  qreal h = frectSize.height();
    path.addEllipse(0,-h/2,w,h);
    path.addPolygon(ftitlePolygon);
    fbrect = path.boundingRect();
}

bool zEllipse::pointIn(QPointF point)
{
    QPainterPath path;
    qreal w = frectSize.width();  qreal h = frectSize.height();
    path.addEllipse(0,-h/2,w,h);
    return path.contains(point);
}

QPolygonF zEllipse::getTitlePolygon()
{
    QMatrix matrix;
    QPolygonF polygon;

    QPointF pcenter = getCenterPoint() / data(1).toDouble();
    QLineF line(QPointF(0,0),pcenter);
    qreal acenter = qAtan2(line.dy(),line.dx());
    qreal R = qDegreesToRadians(rotation()-data(2).toDouble());
    R += acenter;
    pcenter.setX(line.length()*qCos(R));
    pcenter.setY(line.length()*qSin(R));

    QString title = getTitle();
    title = QString(" %1 ").arg(title);
    QFontMetricsF fm(ffont);
    int tw = fm.width(title);
    int th = fm.height();
    QRect rect = QRect(pcenter.toPoint()-QPoint(tw/2,th/2),QSize(tw,th));

    polygon = matrix.mapToPolygon(rect);
    matrix.scale(data(1).toDouble(),data(1).toDouble());
    matrix.rotate(data(2).toDouble()-rotation());

    return matrix.map(polygon);
}

void zEllipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (isSelected()) {
        painter->setPen(selectedPen);
        painter->setBrush(selectedBrush);
    } else {
        painter->setPen(basePen);
        painter->setBrush(baseBrush);
    }
    qreal w = frectSize.width();  qreal h = frectSize.height();
    painter->drawEllipse(0,-h/2,w,h);
    painter->drawPolygon(ftitlePolygon);
    if (isSelected()) {
        painter->drawLine(0,0,w,0);
        painter->drawLine(w/2,-h/2,w/2,h/2);
    }
// наименование объекта
    setOpacity(fOpacity);
    QString title = getTitle();
    painter->setFont(ffont);    painter->setPen(titleRectPen);    painter->setBrush(titleRectBrush);
    painter->save();
    painter->scale(data(1).toDouble(),data(1).toDouble());  // ===
    painter->rotate(data(2).toDouble()-rotation());
    fcenterPoint = getCenterPoint() / data(1).toDouble();
    QLineF line(QPointF(0,0),fcenterPoint);
    qreal R = qDegreesToRadians(rotation()-data(2).toDouble());
    fcenterPoint.setX(line.length()*qCos(R));
    fcenterPoint.setY(line.length()*qSin(R));
    ftitleRect = getTitleRectangle();
    if (isSelected()) painter->setBrush(titleRectBrushSelection);
    else painter->setBrush(titleRectBrush);
    painter->drawRoundRect(ftitleRect,12,25);
    painter->setPen(titleTextPen);
    painter->drawText(ftitleRect,QString(" %1 ").arg(title));
    painter->restore();                                     // ===
    fcenterPoint = getCenterPoint();
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

QStringList zEllipse::getSettings(int num)
{
    QStringList strlist;
    // rectangle7/title#Прямоугольник 1
    strlist.append(QString("Ellipse%1/objectvisible%2%3").arg(num).arg(setsep).arg(this->isVisible()));
    strlist.append(QString("Ellipse%1/title%2%3").arg(num).arg(setsep).arg(getTitle()));
    QPointF point = getRectPoint2(pos(), frectSize.width(), rotation());
    strlist.append(QString("Ellipse%1/pos%2%3,%4").arg(num).arg(setsep)
                   .arg(pos().rx(),0,'f',2).arg(pos().ry(),0,'f',2)
                   /*.arg(point.rx(),0,'f',2).arg(point.ry(),0,'f',2)*/);
    qreal angle = rotation();
    strlist.append(QString("Ellipse%1/rotation%2%3").arg(num).arg(setsep).arg(angle,0,'f',2));
    strlist.append(QString("Ellipse%1/size%2%3,%4").arg(num).arg(setsep)
                   .arg(frectSize.width(),0,'f',2).arg(frectSize.height(),0,'f',2));
    strlist.append(QString("Ellipse%1/dockvisible%2%3").arg(num).arg(setsep).arg(dockw->isVisible()));
    QPoint dwpos = dockw->geometry().topLeft();
    strlist.append(QString("Ellipse%1/dockwpos%2%3,%4").arg(num).arg(setsep)
                   .arg(dwpos.x()).arg(dwpos.y()));
    return strlist;
}

QPointF zEllipse::getCenterPoint()
{
    qreal w = frectSize.width();
    QPointF cp = QPointF(w/2,0);
    return cp;
}

zPolyline::zPolyline(qreal scale, qreal rotate)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setData(1,scale);  setData(2,rotate);
    typeString = "Полилиния";
}

QRectF zPolyline::getTitleRectangle() const
{
    QString title = getTitle();
    title = QString(" %1 ").arg(title);
    QFontMetricsF fm(ffont);
    int tw = fm.width(title);
    int th = fm.height();
    return QRectF(fcenterPoint-QPointF(tw/2,th/2),QSizeF(tw,th));
}

QPainterPath zPolyline::shape() const
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill); // то что надо
// добавляем только окрестности линий
    for( int i = 0; i < fpolygon.length() - 1; i++ ) {
        path.moveTo(fpolygon[i]);
        path.lineTo(fpolygon[i+1]);
    }  // for
    path.addPolygon(ftitlePolygon);
    return path;
}

void zPolyline::updateBoundingRect()
{
    fcenterPoint = getCenterPoint();
    ftitleRect = getTitleRectangle();
    ftitlePolygon = getTitlePolygon();
    QPainterPath path;
    path.setFillRule(Qt::WindingFill); // то что надо
    path.addPolygon(fpolygon);
    path.addPolygon(ftitlePolygon);
    fbrect = path.boundingRect();
}

bool zPolyline::pointIn(QPointF point)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill); // то что надо
// добавляем только окрестности линий
    for( int i = 0; i < fpolygon.length() - 1; i++ ) {
        path.moveTo(fpolygon[i]);
        path.lineTo(fpolygon[i+1]);
    }  // for
//    return path.contains(point);
    return path.intersects(QRectF(point,QSizeF(1,1)));
}

QPolygonF zPolyline::getTitlePolygon()
{
    QMatrix matrix;
    QPolygonF polygon;

    QPointF pcenter = getCenterPoint() / data(1).toDouble();
    QLineF line(QPointF(0,0),pcenter);
    qreal acenter = qAtan2(line.dy(),line.dx());
    qreal R = qDegreesToRadians(-data(2).toDouble());
    R += acenter;
    pcenter.setX(line.length()*qCos(R));
    pcenter.setY(line.length()*qSin(R));

    QString title = getTitle();
    title = QString(" %1 ").arg(title);
    QFontMetricsF fm(ffont);
    int tw = fm.width(title);
    int th = fm.height();
    QRect rect = QRect(pcenter.toPoint()-QPoint(tw/2,th/2),QSize(tw,th));

    polygon = matrix.mapToPolygon(rect);
    matrix.scale(data(1).toDouble(),data(1).toDouble());
    matrix.rotate(data(2).toDouble()-rotation());

    return matrix.map(polygon);
}

void zPolyline::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (isSelected()) {
        painter->setPen(selectedPen);
        painter->setBrush(selectedBrush);
    } else {
        painter->setPen(basePen);
        painter->setBrush(baseBrush);
    }

    painter->drawPolyline(fpolygon);
    painter->drawPolygon(ftitlePolygon);

    // наименование объекта
    setOpacity(fOpacity);
    QString title = getTitle();
    painter->setFont(ffont);    painter->setPen(titleRectPen);    painter->setBrush(titleRectBrush);
//    fcenterPoint = getCenterPoint();
    painter->save();
    painter->scale(data(1).toDouble(),data(1).toDouble());  // ===
    painter->rotate(data(2).toDouble()-rotation());
    fcenterPoint = getCenterPoint() / data(1).toDouble();
    QLineF line(QPointF(0,0),fcenterPoint);
    qreal centerAngle = qAtan2(line.dy(),line.dx());
    qreal R = qDegreesToRadians(-data(2).toDouble());
    R += centerAngle;
    fcenterPoint.setX(line.length()*qCos(R));
    fcenterPoint.setY(line.length()*qSin(R));
    ftitleRect = getTitleRectangle();
    if (isSelected()) painter->setBrush(titleRectBrushSelection);
    else painter->setBrush(titleRectBrush);
    painter->drawRoundRect(ftitleRect,12,25);
    painter->setPen(titleTextPen);
    painter->drawText(ftitleRect,QString(" %1 ").arg(title));
    painter->restore();                                     // ===
    fcenterPoint = getCenterPoint();
//    painter->drawLine(fcenterPoint.x(),fcenterPoint.y()-30,fcenterPoint.x(),fcenterPoint.y()+30);
//    painter->drawLine(fcenterPoint.x()-30,fcenterPoint.y(),fcenterPoint.x()+30,fcenterPoint.y());
//    painter->drawLine(0,0-20,0,0+20);
//    painter->drawLine(0-20,0,0+20,0);

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void zPolyline::addPoint(QPoint point)
{
    fpolygon << point;
}

QStringList zPolyline::getSettings(int num)
{
    QStringList strlist;
    // polygon7/title#Полигон 1
    strlist.append(QString("Polyline%1/objectvisible%2%3").arg(num).arg(setsep).arg(this->isVisible()));
    strlist.append(QString("Polyline%1/title%2%3").arg(num).arg(setsep).arg(getTitle()));
    strlist.append(QString("Polyline%1/pos%2%3,%4").arg(num).arg(setsep)
                   .arg(pos().rx(),0,'f',2).arg(pos().ry(),0,'f',2));
    QString xstr, ystr;
    foreach(QPoint point, fpolygon) {
      xstr.append(QString::number(point.rx(), 'f', 2));
      ystr.append(QString::number(point.ry(), 'f', 2));
      if (fpolygon.indexOf(point) < fpolygon.length()-1) {
        xstr.append(",");
        ystr.append(",");
      }
    }
    strlist.append(QString("Polyline%1/x%2%3").arg(num).arg(setsep).arg(xstr));
    strlist.append(QString("Polyline%1/y%2%3").arg(num).arg(setsep).arg(ystr));
    strlist.append(QString("Polyline%1/dockvisible%2%3").arg(num).arg(setsep).arg(dockw->isVisible()));
    QPoint dwpos = dockw->geometry().topLeft();
    strlist.append(QString("Polyline%1/dockwpos%2%3,%4").arg(num).arg(setsep)
                   .arg(dwpos.x()).arg(dwpos.y()));
    return strlist;
}

QPointF zPolyline::getCenterPoint()
{
    if (fpolygon.isEmpty()) return QPointF(0,0);
    QPointF point;
    qreal x = 0.0;  qreal y = 0.0;
    foreach ( QPoint p , fpolygon ) {
        x += p.rx();  y += p.ry();
    }  // foreach
    point.setX(x/fpolygon.length());
    point.setY(y/fpolygon.length());
    return point;
}

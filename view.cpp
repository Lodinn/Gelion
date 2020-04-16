#include "view.h"

#include <QAction>
#include <QDebug>
#include <QGraphicsScene>
#include <QIcon>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

gQGraphicsView::gQGraphicsView(QGraphicsScene *scene) : QGraphicsView(scene)
{
    const QIcon openIcon = QIcon::fromTheme("Открыть файл...", QIcon(":/icons/open.png"));
    openAct = new QAction(openIcon, tr("&Открыть файл..."), this);
    openAct->setShortcut(QKeySequence::Open);

    const QIcon pointIcon = QIcon::fromTheme("Точка", QIcon(":/images/pin_grey.png"));
    const QIcon rectIcon = QIcon::fromTheme("Прямоугольник", QIcon(":/images/vector_square.png"));
    const QIcon ellipseIcon = QIcon::fromTheme("Эллипс", QIcon(":/images/vector_ellipse.png"));
    const QIcon polylineIcon = QIcon::fromTheme("Полилиния", QIcon(":/images/polyline-64.png"));
    const QIcon polygonIcon = QIcon::fromTheme("Полигон", QIcon(":/images/vector-polygon.png"));

    pointAct = new QAction(pointIcon, "Добавить ОИ-[Точка]", this);
    rectAct = new QAction(rectIcon, "Добавить ОИ-[Прямоугольник]", this);
    ellipseAct = new QAction(ellipseIcon, "Добавить ОИ-[Эллипс]", this);
    polylineAct = new QAction(polylineIcon, "Добавить ОИ-[Полилиния]", this);
    polygonAct = new QAction(polygonIcon, "Добавить ОИ- [Полигон]", this);
    closeAct = new QAction("&Выход", this);
    closeAct->setShortcuts(QKeySequence::Close);
    const QIcon winZGraphListIcon = QIcon::fromTheme("Список Объектов", QIcon(":/icons/windows2.png"));
    winZGraphListAct = new QAction(winZGraphListIcon, "Список Объектов", this);
    winZGraphListAct->setShortcut(QKeySequence::Preferences);
    winZGraphListAct->setCheckable(true); winZGraphListAct->setChecked(true);

    const QIcon channelListIcon = QIcon::fromTheme("Список Каналов", QIcon(":/icons/list-channel.jpg"));
    channelListAct = new QAction(channelListIcon, "Список Каналов", this);
    channelListAct->setCheckable(true); channelListAct->setChecked(true);

    const QIcon ShowAllIcon = QIcon::fromTheme("Показать все профили", QIcon(":/icons/profs_show.png"));
    const QIcon HideAllIcon = QIcon::fromTheme("Скрыть все профили", QIcon(":/icons/profs_hide.png"));
    winZGraphListShowAllAct = new QAction(ShowAllIcon, "Показать все профили", this);
    winZGraphListHideAllAct = new QAction(HideAllIcon, "Скрыть все профили", this);

    connect(pointAct, SIGNAL(triggered()), this, SLOT(setInputModePoint()));
    connect(rectAct, SIGNAL(triggered()), this, SLOT(setInputModeRect()));
    connect(ellipseAct, SIGNAL(triggered()), this, SLOT(setInputModeEllipse()));
    connect(polylineAct, SIGNAL(triggered()), this, SLOT(setInputModePolyline()));
    connect(polygonAct, SIGNAL(triggered()), this, SLOT(setInputModePolygon()));
}

void gQGraphicsView::setInputModePoint()
{
    deleteGrabberRects();
    insertMode = gQGraphicsView::Point;
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

void gQGraphicsView::setInputModeRect()
{
    deleteGrabberRects();
    insertMode = gQGraphicsView::Rect;
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

void gQGraphicsView::setInputModeEllipse()
{
    deleteGrabberRects();
    insertMode = gQGraphicsView::Ellipse;
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

void gQGraphicsView::setInputModePolyline()
{
    deleteGrabberRects();
    insertMode = gQGraphicsView::Polyline;
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

void gQGraphicsView::setInputModePolygon()
{
    deleteGrabberRects();
    insertMode = gQGraphicsView::Polygon;
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

void gQGraphicsView::selectionZChanged()
{
    deleteGrabberRects();
    grabberItem = qgraphicsitem_cast<zGraph *>(scene()->mouseGrabberItem());
    if (grabberItem == nullptr) return;
    grabberItem->dockw->activateWindow();
    switch (grabberItem->type()) {
    case zPoint::Type : {                   // тип объекта - точка             Point
        connect(grabberItem,SIGNAL(mouseMove()),this,SLOT(moveGrabberRects()));
        return;
    }  // case zPoint::Type
    case zRect::Type : {        // тип объекта - прямоугольник Rect 2	с возможностью поворота
        grabberItem->setCursor(QCursor(Qt::SizeAllCursor));
        connect(grabberItem,SIGNAL(mouseMove()),this,SLOT(moveGrabberRects()));
        connect(grabberItem,SIGNAL(mouseRelease()),this,SLOT(releaseGrabberRects()));
        zRect *zR = qgraphicsitem_cast<zRect *>(grabberItem);
        QPointF pos = zR->pos();
        zContourRect *zrect = new zContourRect(pos);                    // первая точка (0)
        zrect->setData(0, 0);  scene()->addItem(zrect); zcRects.append(zrect);
        connect(zrect,SIGNAL(mousePress()),this,SLOT(pressOneGrabberRect()));
        connect(zrect,SIGNAL(mouseMove(int,QPointF)),this,SLOT(moveOneGrabberRect(int,QPointF)));
        connect(zrect,SIGNAL(mouseRelease()),this,SLOT(releaseOneGrabberRect()));
        zrect = new zContourRect(pos);                                  // вторая точка (1)
        zrect->setData(0, 1);  scene()->addItem(zrect); zcRects.append(zrect);
        connect(zrect,SIGNAL(mousePress()),this,SLOT(pressOneGrabberRect()));
        connect(zrect,SIGNAL(mouseMove(int,QPointF)),this,SLOT(moveOneGrabberRect(int,QPointF)));
        connect(zrect,SIGNAL(mouseRelease()),this,SLOT(releaseOneGrabberRect()));
        setGrabberCoordTozRect();
        return;
    }  // case zRect::Type
    case zEllipse::Type : {        // тип объекта - эллипс Ellipse  3	с возможностью поворота
        grabberItem->setCursor(QCursor(Qt::SizeAllCursor));
        connect(grabberItem,SIGNAL(mouseMove()),this,SLOT(moveGrabberRects()));
        connect(grabberItem,SIGNAL(mouseRelease()),this,SLOT(releaseGrabberRects()));
        zEllipse *zE = qgraphicsitem_cast<zEllipse *>(grabberItem);
        QPointF pos = zE->pos();
        zContourRect *zrect = new zContourRect(pos);                    // первая точка (0)
        zrect->setData(0, 0);  scene()->addItem(zrect); zcRects.append(zrect);
        connect(zrect,SIGNAL(mousePress()),this,SLOT(pressOneGrabberRect()));
        connect(zrect,SIGNAL(mouseMove(int,QPointF)),this,SLOT(moveOneGrabberRect(int,QPointF)));
        connect(zrect,SIGNAL(mouseRelease()),this,SLOT(releaseOneGrabberRect()));
        zrect = new zContourRect(pos);                                  // вторая точка (1)
        zrect->setData(0, 1);  scene()->addItem(zrect); zcRects.append(zrect);
        connect(zrect,SIGNAL(mousePress()),this,SLOT(pressOneGrabberRect()));
        connect(zrect,SIGNAL(mouseMove(int,QPointF)),this,SLOT(moveOneGrabberRect(int,QPointF)));
        connect(zrect,SIGNAL(mouseRelease()),this,SLOT(releaseOneGrabberRect()));
        setGrabberCoordTozEllipse();
        return;
    }  // case zEllipse::Type
    case zPolyline::Type : {             // тип объекта - полилиния Polyline  4
        grabberItem->setCursor(QCursor(Qt::SizeAllCursor));
        connect(grabberItem,SIGNAL(mouseMove()),this,SLOT(moveGrabberRects()));
        connect(grabberItem,SIGNAL(mouseRelease()),this,SLOT(releaseGrabberRects()));
        zPolyline *zp = qgraphicsitem_cast<zPolyline *>(grabberItem);
        foreach (QPoint point, zp->fpolygon) {
            QPointF pos = zp->pos();
            QPointF zpoint(pos.rx()+point.rx(), pos.ry()+point.ry());
            zContourRect *zrect = new zContourRect(zpoint);
            zrect->setData(0, zp->fpolygon.indexOf(point));
            scene()->addItem(zrect);
            zcRects.append(zrect);
            connect(zrect,SIGNAL(mousePress()),this,SLOT(pressOneGrabberRect()));
            connect(zrect,SIGNAL(mouseMove(int,QPointF)),this,SLOT(moveOneGrabberRect(int,QPointF)));
            connect(zrect,SIGNAL(mouseRelease()),this,SLOT(releaseOneGrabberRect()));
        }
        return;
    }  // case zPolyline::Type
    case zPolygon::Type : {             // тип объекта - область(полигон) Polygon  5
        grabberItem->setCursor(QCursor(Qt::SizeAllCursor));
        connect(grabberItem,SIGNAL(mouseMove()),this,SLOT(moveGrabberRects()));
        connect(grabberItem,SIGNAL(mouseRelease()),this,SLOT(releaseGrabberRects()));
        zPolygon *zp = qgraphicsitem_cast<zPolygon *>(grabberItem);
        foreach (QPoint point, zp->fpolygon) {
            QPointF pos = zp->pos();
            QPointF zpoint(pos.rx()+point.rx(), pos.ry()+point.ry());
            zContourRect *zrect = new zContourRect(zpoint);
            zrect->setData(0, zp->fpolygon.indexOf(point));
            scene()->addItem(zrect);
            zcRects.append(zrect);
            connect(zrect,SIGNAL(mousePress()),this,SLOT(pressOneGrabberRect()));
            connect(zrect,SIGNAL(mouseMove(int,QPointF)),this,SLOT(moveOneGrabberRect(int,QPointF)));
            connect(zrect,SIGNAL(mouseRelease()),this,SLOT(releaseOneGrabberRect()));
        }
        return;
    }  // case zPolygon::Type
    }  // switch (grabberItem->type())
}

void gQGraphicsView::setGrabberCoordTozRect()
{
    if (grabberItem == nullptr) return;
    zRect *zR = qgraphicsitem_cast<zRect *>(grabberItem);
    QPointF pos = zR->pos();
    qreal w = zR->frectSize.width();
    qreal degrees = zR->rotation();
    double radians = qDegreesToRadians(degrees);
    QPointF pos2 = QPointF(pos.rx()+w*qCos(radians),pos.ry()+w*qSin(radians));
    zcRects[0]->setPos(pos);
    zcRects[1]->setPos(pos2);
}

void gQGraphicsView::setGrabberCoordTozEllipse()
{
    if (grabberItem == nullptr) return;
    zEllipse *zE = qgraphicsitem_cast<zEllipse *>(grabberItem);
    QPointF pos = zE->pos();
    qreal w = zE->frectSize.width();
    qreal degrees = zE->rotation();
    double radians = qDegreesToRadians(degrees);
    QPointF pos2 = QPointF(pos.rx()+w*qCos(radians),pos.ry()+w*qSin(radians));
    zcRects[0]->setPos(pos);
    zcRects[1]->setPos(pos2);
}

void gQGraphicsView::pressOneGrabberRect()
{
    if (grabberItem == nullptr) return;
    grabberItem->setFlag(QGraphicsItem::ItemIsMovable, false);
    grabberItem->dockw->activateWindow();
}

void gQGraphicsView::releaseOneGrabberRect()
{
    if (grabberItem == nullptr) return;
    show_profile_for_Z(grabberItem);
    grabberItem->setFlag(QGraphicsItem::ItemIsMovable, true);
}

void gQGraphicsView::moveOneGrabberRect(int num, QPointF point)
{
    if (grabberItem == nullptr) return;
    switch (grabberItem->type()) {
    case zRect::Type : {    // тип объекта - прямоугольник Rect 2	с возможностью поворота
        zRect *zR = qgraphicsitem_cast<zRect *>(grabberItem);
        if (num == 0) {
            zR->setPos(point);
            QPointF delta = zcRects[1]->pos() - zcRects[0]->pos();
            QLineF line(zcRects[1]->pos(),zcRects[0]->pos());
            qreal angle = qAtan2(delta.ry(),delta.rx()) * 180 / 3.1415;
            zR->setRotation(angle);
            zR->frectSize.setWidth(line.length());
            zR->updateBoundingRect();
            zR->update();
        }  // if (num == 0)
        if (num == 1) {
            QPointF delta = zcRects[1]->pos() - zcRects[0]->pos();
            QLineF line(zcRects[1]->pos(),zcRects[0]->pos());
            qreal angle = qRadiansToDegrees(qAtan2(delta.ry(),delta.rx()));
            zR->setRotation(angle);
            zR->frectSize.setWidth(line.length());
            zR->updateBoundingRect();
            zR->update();
        }  // if (num == 1)
        break;
    }  // case zRect::Type
    case zEllipse::Type : {  // тип объекта - эллипс Ellipse  3	с возможностью поворота
        zEllipse *zE = qgraphicsitem_cast<zEllipse *>(grabberItem);
        if (num == 0) {
            zE->setPos(point);
            QPointF delta = zcRects[1]->pos() - zcRects[0]->pos();
            QLineF line(zcRects[1]->pos(),zcRects[0]->pos());
            qreal angle = qAtan2(delta.ry(),delta.rx()) * 180 / 3.1415;
            zE->setRotation(angle);
            zE->frectSize.setWidth(line.length());
            zE->updateBoundingRect();
            zE->update();
        }  // if (num == 0)
        if (num == 1) {
            QPointF delta = zcRects[1]->pos() - zcRects[0]->pos();
            QLineF line(zcRects[1]->pos(),zcRects[0]->pos());
            qreal angle = qRadiansToDegrees(qAtan2(delta.ry(),delta.rx()));
            zE->setRotation(angle);
            zE->frectSize.setWidth(line.length());
            zE->updateBoundingRect();
            zE->update();
        }  // if (num == 1)
        break;
    }  // case zEllipse::Type
    case zPolyline::Type : {               // тип объекта - полилиния Polyline  4
        zPolyline *zp = qgraphicsitem_cast<zPolyline *>(grabberItem);
        QPointF pos = zp->pos();
        zp->fpolygon[num].setX(point.rx() - pos.rx());
        zp->fpolygon[num].setY(point.ry() - pos.ry());
        zp->updateBoundingRect();
        zp->update();
        zcRects[num]->update();
        break;
    }  // case zPolygon::Type
    case zPolygon::Type : {             // тип объекта - область(полигон) Polygon 5
        zPolygon *zp = qgraphicsitem_cast<zPolygon *>(grabberItem);
        QPointF pos = zp->pos();
        zp->fpolygon[num].setX(point.rx() - pos.rx());
        zp->fpolygon[num].setY(point.ry() - pos.ry());
        zp->updateBoundingRect();
        zp->update();
        zcRects[num]->update();
        break;
    }  // case zPolygon::Type
    default:
        break;
    }  // switch
}

QList<zGraph *> gQGraphicsView::getZGraphItemsList()
{
    QList<zGraph *> zglist;
    QList<QGraphicsItem *> items = scene()->items();
    foreach(QGraphicsItem *it, items)
        switch (it->type()) {
        case zPoint::Type : { zglist.append(qgraphicsitem_cast<zGraph *>(it)); break; }
        case zRect::Type : { zglist.append(qgraphicsitem_cast<zGraph *>(it)); break; }
        case zEllipse::Type : { zglist.append(qgraphicsitem_cast<zGraph *>(it)); break; }
        case zPolyline::Type : { zglist.append(qgraphicsitem_cast<zGraph *>(it)); break; }
        case zPolygon::Type : { zglist.append(qgraphicsitem_cast<zGraph *>(it)); break; }
        default: continue;
        }  //  switch
    return zglist;
}

void gQGraphicsView::clearZGraphItemsList()
{
    auto graphList = getZGraphItemsList();
    foreach (zGraph *zg, graphList) {
        zg->dockw->setObjectName("");
        zg->dockw->deleteLater();
        scene()->removeItem(zg);
        delete zg;
    }  // foreach
}

void gQGraphicsView::show_profile_for_Z(zGraph *item)
{
    switch (item->type()) {
    case zPoint::Type : {
        int d = imgHand->current_image()->get_bands_count();
        QVector<double> x(d), y(d);
        QVector<QPointF> v = imgHand->current_image()->get_profile(item->pos().toPoint());
        if (v.isEmpty()) return;
        if (v.length() != d) return;
        for (int i = 0; i < d; ++i) {
          x[i] = v[i].rx();   y[i] = v[i].ry();
        }  // for
        item->plot->graph(0)->setData(x, y);
        item->plot->replot();
        return;
    }  // case zPoint::Type
    case zRect::Type : { multiPointsReplotRect(item);   return; }  // case zRect::Type
    case zEllipse::Type : { multiPointsReplotRect(item);  return; }  // case zEllipse::Type
    case zPolyline::Type : { multiPointsReplotPoly(item);  return; }  // case zPolyline::Type
    case zPolygon::Type : { multiPointsReplotPoly(item);  return; }  // case zPolygon::Type
    }  // switch
}

void gQGraphicsView::multiPointsReplotRect(zGraph *item) {

    int d = imgHand->current_image()->get_bands_count();
    QVector<double> x(d), y(d);
    for (int i; i<d; i++) { x[i]=0.0;  y[i]=0.0; }
    int count = 0;
    QRectF rect = item->boundingRect();
    for (int i=rect.left(); i<rect.left()+rect.width()+1; i++)
        for (int j=rect.top(); j<rect.top()+rect.height()+1; j++) {
            if (item->pointIn(QPointF(i,j))) {
                QLineF line = QLineF(0,0,i,j);
                qreal len = line.length();
                qreal a0 = qAtan2(j,i);
                qreal a1 =  qDegreesToRadians(item->rotation());
                qreal a2 = a0 + a1;
                int px = item->pos().x() + len * qCos(a2);
                int py = item->pos().y() + len * qSin(a2);
                QVector<QPointF> v = imgHand->current_image()->get_profile(QPoint(px, py));
                if (v.isEmpty()) continue;
                if (v.length() != d) continue;
                if (count == 0) for (int i = 0; i < d; ++i) x[i] = v[i].rx();
                for (int i = 0; i < d; ++i) y[i] = y[i] + v[i].ry();
                count++;
            }  // if
        }  // for
    if (count > 0) for (int i = 0; i < d; ++i) y[i] = y[i] / count;
    else return;
    item->plot->graph(0)->setData(x, y);
    item->plot->replot();
}

void gQGraphicsView::multiPointsReplotPoly(zGraph *item)
{
    int d = imgHand->current_image()->get_bands_count();
    QVector<double> x(d), y(d);
    for (int i; i<d; i++) { x[i]=0.0;  y[i]=0.0; }
    int count = 0;
    item->updateBoundingRect();
    QRectF rect = item->boundingRect();
    for (int i=rect.left(); i<rect.left()+rect.width()+1; i++)
        for (int j=rect.top(); j<rect.top()+rect.height()+1; j++) {
            if (item->pointIn(QPointF(i,j))) {
                int px = item->pos().x() + i;
                int py = item->pos().y() + j;
                QVector<QPointF> v = imgHand->current_image()->get_profile(QPoint(px, py));
                if (v.isEmpty()) { qDebug() << count; continue; }
                if (v.length() != d) { qDebug() << count; continue; }
                if (count == 0) for (int i = 0; i < d; ++i) x[i] = v[i].rx();
                for (int i = 0; i < d; ++i) y[i] = y[i] + v[i].ry();
                count++;
            }  // if
        }  // for
    if (count > 0) for (int i = 0; i < d; ++i) y[i] = y[i] / count;
    else return;
    item->plot->graph(0)->setData(x, y);
    item->plot->replot();
}

void gQGraphicsView::moveGrabberRects()
{
    if (grabberItem == 0) return;
    switch (grabberItem->type()) {
    case zPoint::Type : {  // тип объекта - точка             Point
        show_profile_for_Z(grabberItem);
        break;
    }  // case zPoint::Type
    case zRect::Type : {        // тип объекта - прямоугольник  Rect 2 с возможностью поворота
        setGrabberCoordTozRect();
//        show_profile_for_Z(grabberItem);
        break;
    }  // case zRect::Type
    case zEllipse::Type : {        // тип объекта - эллипс Ellipse  3	с возможностью поворота
        setGrabberCoordTozEllipse();
//        show_profile_for_Z(grabberItem);
        break;
    }  // case zEllipse::Type
    case zPolyline::Type : {             // тип объекта - полилиния Polyline  4
        zPolyline *zp = qgraphicsitem_cast<zPolyline *>(grabberItem);
        foreach (QPoint point, zp->fpolygon) {
            QPointF pos = zp->pos();
            zcRects[zp->fpolygon.indexOf(point)]->setPos(pos.rx()+point.rx(), pos.ry()+point.ry());
           }
//        show_profile_for_Z(grabberItem);
        break;
    }  // case zPolyline::Type
    case zPolygon::Type : {             // тип объекта - область(полигон)  Polygon       5
        zPolygon *zp = qgraphicsitem_cast<zPolygon *>(grabberItem);
        foreach (QPoint point, zp->fpolygon) {
            QPointF pos = zp->pos();
            zcRects[zp->fpolygon.indexOf(point)]->setPos(pos.rx()+point.rx(), pos.ry()+point.ry());
           }
//        show_profile_for_Z(grabberItem);
        break;
    }  // case zPolygon::Type
    }  // switch
}

void gQGraphicsView::releaseGrabberRects()
{
    if (grabberItem == 0) return;
    show_profile_for_Z(grabberItem);
}

void gQGraphicsView::deleteGrabberRects()
{
    foreach (zContourRect *rect, zcRects) {
        rect->disconnect(); scene()->removeItem(rect);
        delete rect;
    }  // foreach
    zcRects.clear();
    if (grabberItem == nullptr) return;
    grabberItem->disconnect();
}

void gQGraphicsView::deleteTmpLines()
{
    foreach (QGraphicsLineItem *line, tmpLines) {
        scene()->removeItem(line);
        delete line;
    }  // foreach
    tmpLines.clear();
}

QPixmap gQGraphicsView::changeBrightnessPixmap(QImage &img, qreal brightness)
{
    QColor color;
    int h, s, v;
    qreal t_brightnessValue = brightness;
    for(int row = 0; row < img.height(); ++row) {
        unsigned int * data = (unsigned int *) img.scanLine(row);
        for(int col = 0; col < img.width(); ++col) {
            unsigned int & pix = data[col];
            color.setRgb(qRed(pix), qGreen(pix), qBlue(pix));
            color.getHsv(&h, &s, &v);
            v *= t_brightnessValue; // значение, на которое надо увеличить яркость
            v = qMin(v, 255);
            color.setHsv(h, s, v);
// сохраняем изменённый пиксель
            pix = qRgba(color.red(), color.green(), color.blue(), qAlpha(pix));
        }  // for
    }  // for
    return QPixmap::fromImage(img);
}

void gQGraphicsView::setScaleItems(qreal &newscale)
{
    foreach (QGraphicsItem *it , scene()->items()) {
        it->setData(1, newscale);
        if (it->type() != QGraphicsPixmapItem::Type)
            qgraphicsitem_cast<zGraph *>(it)->updateBoundingRect();
    }  // if

}

void gQGraphicsView::setRotateItems(qreal &newangle)
{
    foreach (QGraphicsItem *it , scene()->items()) {
        it->setData(2, newangle);
        if (it->type() != QGraphicsPixmapItem::Type)
            qgraphicsitem_cast<zGraph *>(it)->updateBoundingRect();
    }  // if
}

void gQGraphicsView::wheelEvent(QWheelEvent *event)
{
    QList<QGraphicsItem *> selection = scene()->selectedItems();
    if (selection.length() > 0) {
        zGraph *graph  = qgraphicsitem_cast<zGraph *>(selection[0]);
        switch (graph->type()) {
        case zPoint::Type : return;
        case zRect::Type : {        // тип объекта - прямоугольник Rect 2	с возможностью поворота
            zRect *zR = qgraphicsitem_cast<zRect *>(graph);
            qreal h = 0.0;
            if(event->delta() > 0) h = 1;
            else h = - 1;
            qreal newheight = zR->frectSize.height()+h;
            if (newheight >= 0.0)
                zR->frectSize.setHeight(newheight);
            zR->updateBoundingRect();
            zR->update();
            show_profile_for_Z(zR);
            return;
        }  // case zRect::Type
        case zEllipse::Type : {          // тип объекта - эллипс Ellipse  3	с возможностью поворота
            zEllipse *zE = qgraphicsitem_cast<zEllipse *>(graph);
            qreal h = 0;
            if(event->delta() > 0) h = 1;
            else h = - 1;
            qreal newheight = zE->frectSize.height()+h;
            if (newheight >= 0)
                zE->frectSize.setHeight(newheight);
            zE->updateBoundingRect();
            zE->update();
            show_profile_for_Z(zE);
            return;
        }  // case zRect::Type
        case zPolyline::Type : return;
        case zPolygon::Type : return;
        }  // switch (graph->type())
    }  // if (selection.length() > 0)
    switch (insertMode) {
    case gQGraphicsView::None : break;
    case gQGraphicsView::Point : return;
    case gQGraphicsView::Rect : {
        if (tmpRect == nullptr) break;
        qreal h = 0;
        if(event->delta() > 0) h = 1;
        else h = - 1;
        qreal newheight = tmpRect->frectSize.height()+h;
        if (newheight >= 0.0)
            tmpRect->frectSize.setHeight(newheight);
        tmpRect->updateBoundingRect();
        tmpRect->update();
        return;
    }  // case gQGraphicsView::Rect
    case gQGraphicsView::Ellipse : {
        if (tmpEllipse == nullptr) break;
        qreal h = 0;
        if(event->delta() > 0) h = 1;
        else h = - 1;
        qreal newheight = tmpEllipse->frectSize.height()+h;
        if (newheight >= 0.0)
            tmpEllipse->frectSize.setHeight(newheight);
        tmpEllipse->updateBoundingRect();
        tmpEllipse->update();
        return;
    }  // case gQGraphicsView::Ellipse
    case gQGraphicsView::Polyline : return;
    case gQGraphicsView::Polygon : return;
    }  // switch (insertMode)
    if (event->modifiers() == Qt::ControlModifier) {
        qreal sx;
        if(event->delta() > 0) sx = 1.05;
        else sx = 0.95;
        scale(sx, sx);
        GlobalScale /= sx;
        setScaleItems(GlobalScale);
        return;
    }  // if
    if (event->modifiers() == Qt::ShiftModifier) {
        qreal angle;
        if(event->delta() > 0) angle = -1.0;
        else angle = 1.0;
        rotate(angle);
        GlobalRotate -= angle;
        setRotateItems(GlobalRotate);
        return;
    }  // if
    QGraphicsView::wheelEvent(event);
}

void gQGraphicsView:: mousePressEvent(QMouseEvent *event)
{
  QGraphicsView::mousePressEvent(event);
  if (event->modifiers() == Qt::ControlModifier)
      insertMode = gQGraphicsView::Point;
  switch (insertMode) {
  case gQGraphicsView::None : {
      if (scene()->mouseGrabberItem() != nullptr) return;
      if (event->button() != Qt::LeftButton) return;
      PAN = true;
      pX0 = event->x();
      pY0 = event->y();
      setCursor(Qt::ClosedHandCursor);
      event->accept();
      return;
    }  // case gQGraphicsView::None
//    break;
  case gQGraphicsView::Point : {            // тип объекта - точка             Point
      if (event->button() != Qt::LeftButton) return;
      insPoint(event->pos());
      insertMode = gQGraphicsView::None;
      setCursor(Qt::ArrowCursor);
      setMouseTracking(false);
    }  // case gQGraphicsView::Point
    return;
  case gQGraphicsView::Rect : {         // тип объекта - прямоугольник Rect 2	с возможностью поворота
      if (event->button() != Qt::LeftButton) return;
      if (tmpLines.isEmpty()) {
        QPointF p = mapToScene(event->pos());
        qreal x1 = p.rx();  qreal y1 = p.ry();
        qreal x2 = p.rx();  qreal y2 = p.ry();
        tmpLines.append(scene()->addLine(x1,y1,x2,y2,fInsPen));
        tmpRect = new zRect(p,GlobalScale,GlobalRotate);
        tmpRect->setTitle("Прямоугольник 1");
        tmpRect->aicon = rectAct->icon();
        tmpRect->updateBoundingRect();
        scene()->addItem(tmpRect);
        emit insertZGraphItem(tmpRect);
        return;
      } else {
        tmpRect->updateBoundingRect();
        tmpRect->dockw->show();
        show_profile_for_Z(tmpRect);
        deleteTmpLines();
        insertMode = gQGraphicsView::None;
        setCursor(Qt::ArrowCursor);
        setMouseTracking(false);
        emit setZGraphDockToggled(tmpRect);
        tmpRect = nullptr;
        return;
      }  // if (tmpLines.isEmpty())
    }  // case gQGraphicsView::Rect
    break;
  case gQGraphicsView::Ellipse : {          // тип объекта - эллипс Ellipse  3	с возможностью поворота
        if (event->button() != Qt::LeftButton) return;
        if (tmpLines.isEmpty()) {
            QPointF p = mapToScene(event->pos());
            qreal x1 = p.rx();  qreal y1 = p.ry();
            qreal x2 = p.rx();  qreal y2 = p.ry();
            tmpLines.append(scene()->addLine(x1,y1,x2,y2,fInsPen));
            tmpEllipse = new zEllipse(p,GlobalScale,GlobalRotate);
            tmpEllipse->setTitle("Эллипс 1");  tmpEllipse->aicon = QIcon(":/images/vector_ellipse.png");
            tmpEllipse->updateBoundingRect();
            scene()->addItem(tmpEllipse);
            emit insertZGraphItem(tmpEllipse);
            return;
        } else {
            tmpEllipse->updateBoundingRect();
            tmpEllipse->dockw->show();
            show_profile_for_Z(tmpEllipse);
            deleteTmpLines();
            insertMode = gQGraphicsView::None;
            setCursor(Qt::ArrowCursor);
            setMouseTracking(false);
            emit setZGraphDockToggled(tmpEllipse);
            tmpEllipse = nullptr;
            return;
        }  // if (tmpLines.isEmpty())
    }  // case gQGraphicsView::Ellipse
  case gQGraphicsView::Polyline : {          // тип объекта - полилиния Polyline  4
      setCursor(Qt::CrossCursor);
      contextMenuEnable = false;
      if (event->button() == Qt::LeftButton) {
          QPointF p = mapToScene(event->pos());
          qreal x1 = p.rx();  qreal y1 = p.ry();
          qreal x2 = p.rx();  qreal y2 = p.ry();
          tmpLines.append(scene()->addLine(x1,y1,x2,y2,fInsPen));
          return;
      };  // Qt::LeftButton
      if (event->button() == Qt::RightButton) {
          createPolyline();
          insertMode = gQGraphicsView::None;
          setCursor(Qt::ArrowCursor);
          setMouseTracking(false);
          return;
      };  // Qt::RightButton
      return; //TODO: check
  }  // case gQGraphicsView::Polygon
  case gQGraphicsView::Polygon : {          // тип объекта - область(полигон) Polygon 5
      setCursor(Qt::CrossCursor);
      contextMenuEnable = false;
      if (event->button() == Qt::LeftButton) {
          QPointF p = mapToScene(event->pos());
          qreal x1 = p.rx();  qreal y1 = p.ry();
          qreal x2 = p.rx();  qreal y2 = p.ry();
          tmpLines.append(scene()->addLine(x1,y1,x2,y2,fInsPen));
          return;
      };  // Qt::LeftButton
      if (event->button() == Qt::RightButton) {
          createPolygon();
          insertMode = gQGraphicsView::None;
          setCursor(Qt::ArrowCursor);
          setMouseTracking(false);
          return;
      };  // Qt::RightButton
      return;
  }  // case gQGraphicsView::Polygon
  }  // switch (insertMode)
}

void gQGraphicsView::createPolygon()
{
    zPolygon *zp = new zPolygon(GlobalScale,GlobalRotate);
    zp->setTitle("Полигон 1");  zp->aicon = QIcon(":/images/vector-polygon.png");
    foreach (QGraphicsLineItem *item, tmpLines) {
        QPointF point = item->line().p1();
        zp->fpolygon.append(point.toPoint());
    }
    zp->fpolygon.append(tmpLines[tmpLines.length()-1]->line().p2().toPoint());
    zp->updateBoundingRect();
    scene()->addItem(zp);
    emit insertZGraphItem(zp);
    zp->dockw->show();
    show_profile_for_Z(zp);
    deleteTmpLines();
    emit setZGraphDockToggled(zp);
}

void gQGraphicsView::createPolyline()
{
    zPolyline *zp = new zPolyline(GlobalScale,GlobalRotate);
    zp->setTitle("Полилиния 1");  zp->aicon = QIcon(":/images/polyline-64.png");
    foreach (QGraphicsLineItem *item, tmpLines) {
        QPointF point = item->line().p1();
        zp->fpolygon.append(point.toPoint());
    }  // foreach
    int count = tmpLines.length()-1;
    zp->fpolygon.append(tmpLines[count]->line().p2().toPoint());
    zp->updateBoundingRect();
    scene()->addItem(zp);
    emit insertZGraphItem(zp);
    zp->dockw->show();
    show_profile_for_Z(zp);
    deleteTmpLines();
    emit setZGraphDockToggled(zp);
}
  /*qDebug() << scene()->mouseGrabberItem();
  if (scene()->mouseGrabberItem() != nullptr) {
    qDebug() << "Grabbed something";
    QGraphicsView::mousePressEvent(event);
    return;
  }

  if (event->modifiers() == Qt::ControlModifier)
    if (event->button() == Qt::LeftButton) {
      QPointF point = mapToScene(event->pos());
      ROI *new_roi =
          new ROI(point, QString("Point %1").arg(ROIs.count() + 1), ROI::Point);
      scene()->addItem(new_roi);
      ROIs_new.append(new_roi);
      //      scene()->addRect(point.x(), point.y(), 36, 18);
      //      QPainterPath pp;
      //      pp.moveTo(point);
      //      pp.addEllipse(point, 6, 6);
      //      pp.addText(point.x(), point.y(), QFont("Arial", 12),
      //                 QString("Point %1").arg(ROIs.count() + 1));
      //      qDebug() << pp.boundingRect();
      //      QGraphicsPathItem *path =
      //          scene()->addPath(pp, QPen(Qt::yellow), QBrush(Qt::yellow));
      //      path->setFlag(QGraphicsItem::ItemIsMovable);
      //      path->setFlag(QGraphicsItem::ItemIgnoresTransformations);
      //      ROIs.append(static_cast<QGraphicsItem *>(path));
      emit point_picked(point);
      return;
    }

  if (event->button() == Qt::LeftButton) {
    _pan = true;
    _panX0 = event->x();
    _panY0 = event->y();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
    return;
  }
  //    event->ignore();*/

void gQGraphicsView::mouseMoveEvent(QMouseEvent *event) {
  if (PAN) {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() -
                                    (event->x() - pX0));
    verticalScrollBar()->setValue(verticalScrollBar()->value() -
                                  (event->y() - pY0));
    pX0 = event->x();
    pY0 = event->y();
    event->accept();
    return;
  }  // if (PAN)
//  if (event->modifiers() == Qt::ControlModifier) {
//      setMouseTracking(true);
//      setCursor(Qt::CrossCursor);
//   }   else
//      setMouseTracking(false);
  switch (insertMode) {
  case gQGraphicsView::Rect : {         // тип объекта - прямоугольник Rect 2	с возможностью поворота
      if (tmpRect == nullptr) return;
      if (tmpLines.isEmpty()) return;
      QPointF point = mapToScene(event->pos());
      QLineF newLine(tmpLines[0]->line().p1(), point);
      tmpLines[0]->setLine(newLine);
      QPointF p1 = newLine.p1();
      qreal dx  = point.x() - p1.x();
      qreal dy  = point.y() - p1.y();
      qreal angle = qAtan2(dy,dx) * 180 / 3.1415;
      tmpRect->setRotation(angle);
      qreal w = newLine.length();
      tmpRect->frectSize.setWidth(w);
      tmpRect->updateBoundingRect();
      tmpRect->update();
  }  // case gQGraphicsView::Rect
  case gQGraphicsView::Ellipse : {         // тип объекта - эллипс Ellipse  3	с возможностью поворота
      if (tmpEllipse == nullptr) return;
      if (tmpLines.isEmpty()) return;
      QPointF point = mapToScene(event->pos());
      QLineF newLine(tmpLines[0]->line().p1(), point);
      tmpLines[0]->setLine(newLine);
      QPointF p1 = newLine.p1();
      qreal dx  = point.x() - p1.x();
      qreal dy  = point.y() - p1.y();
      qreal angle = qAtan2(dy,dx) * 180 / 3.1415;
      tmpEllipse->setRotation(angle);
      qreal w = newLine.length();
      tmpEllipse->frectSize.setWidth(w);
      tmpEllipse->updateBoundingRect();
      tmpEllipse->update();
  }  // case gQGraphicsView::Ellipse
  case gQGraphicsView::Polyline : {          // тип объекта - полилиния Polyline  4
      if (tmpLines.isEmpty()) return;
      QGraphicsLineItem *line = tmpLines[tmpLines.length()-1];
      QLineF newLine(line->line().p1(), mapToScene(event->pos()));
      line->setLine(newLine);
      return;
  }  // case gQGraphicsView::Polyline
  case gQGraphicsView::Polygon : {          // тип объекта - область(полигон) Polygon 5
      if (tmpLines.isEmpty()) return;
      QGraphicsLineItem *line = tmpLines[tmpLines.length()-1];
      QLineF newLine(line->line().p1(), mapToScene(event->pos()));
      line->setLine(newLine);
      return;
  }  // case gQGraphicsView::Polygon
  default:
      break;
  }  // switch (insertMode)
   QGraphicsView::mouseMoveEvent(event);
}

//  if (scene()->mouseGrabberItem() != nullptr) {
//    for (int i = 0; i < ROIs_new.count(); i++) {
//      if (ROIs_new[i] == scene()->mouseGrabberItem()) {
//        //        ROIs_new[i]->scenePos();
//        auto new_pos = mapToScene(event->pos());
//        ROIs_new[i]->update_position(new_pos);
//        emit roi_position_updated(QPair<int, QPointF>(i, new_pos));
//      }
//    }
//  }
//  QGraphicsView::mouseMoveEvent(event);

void gQGraphicsView::mouseReleaseEvent(QMouseEvent *event) {
  if (PAN)
    if (event->button() == Qt::LeftButton) {
      PAN = false;
      setCursor(Qt::ArrowCursor);
      event->accept();
      return;
    }
  QGraphicsView::mouseReleaseEvent(event);
}

void gQGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!contextMenuEnable) {
        contextMenuEnable = true;  return;
    }  // if
    QMenu menu(this);
    menu.addAction(openAct);
    menu.addSeparator();
    menu.addAction(pointAct);
    menu.addAction(rectAct);
    menu.addAction(ellipseAct);
    menu.addAction(polylineAct);
    menu.addAction(polygonAct);
    menu.addSeparator();
    menu.addAction(closeAct);
    menu.exec(event->globalPos());
}

void gQGraphicsView::insPoint(QPoint pos)
{
    QPointF point = mapToScene(pos);
    zPoint *zpoint = new zPoint(point);
    zpoint->setTitle("Точка 1");  zpoint->aicon = QIcon(":/images/pin_grey.png");
    zpoint->updateBoundingRect();
    scene()->addItem(zpoint);
    emit insertZGraphItem(zpoint);
    zpoint->dockw->show();
    show_profile_for_Z(zpoint);
    emit setZGraphDockToggled(zpoint);
}

ROI::ROI(QPointF position, QString l, shape_t sh)
    : shape(sh), pos(position), label(l) {
  setFlag(ItemIsSelectable, true);
  setFlag(ItemIsMovable);
}

QRectF ROI::boundingRect() const {
  QSizeF point_size = QSizeF(6, 6);
  switch (shape) {
    case Point:
      return QRectF(
          pos - QPointF(point_size.width() / 2, point_size.height() / 2),
          point_size);
    // FIXME: not implemented
    case Line:
    case Circle:
    case Rectangle:
    case Polygon:
      return QRectF(pos, point_size);
  }
}

void ROI::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget) {
  QRectF rect = boundingRect();
  auto offset = rect.size() / 2.0;
  painter->translate(pos);
  QPen pen(Qt::red, 6);
  painter->setFont(QFont("Arial", 12));
  painter->drawText(20, 20, label);
  painter->setPen(pen);
  switch (shape) {
    case Point:
    case Circle:
      painter->drawEllipse(
          QRectF(QPointF(0, 0) - QPointF(offset.width(), offset.height()),
                 rect.size()));
      break;
    case Line:
    case Rectangle:
    case Polygon:
      painter->drawRect(rect);
      break;
  }
  Q_UNUSED(option)
  Q_UNUSED(widget)
}

void ROI::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mousePressEvent(event);
}

void ROI::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mouseMoveEvent(event);
}

void ROI::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mouseReleaseEvent(event);
}

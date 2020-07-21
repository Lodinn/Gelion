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

QString zGraph::getTitleExt() const
{
    if (this->type()== zPoint::Type)
        return this->getTitle();
    else
        return QString("%1 [точек-%2 || ср.кв.отклонение-%3%]").arg(this->getTitle())
                                  .arg(count_of_point).arg(sigma, 0, 'f', 1);
}

void zGraph::setFont(QString family, int pointSize, int weight, bool italic)
{
    ffont = QFont(family, pointSize, weight, italic);
}

void zGraph::setContextMenuConnection()
{
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
}

void zGraph::setSettingsFromFile(QSettings *settings)
{
    switch (this->type()) {
    case zPoint::Type : {
        setSettingsBase(settings);
        graphSetData();
        return;
    } // case zPoint::Type
    case zRect::Type : {
        setSettingsBase(settings);
        setSettingsDeviation(settings);
        graphSetData();
        return;
    }  // case zRect::Type
    case zEllipse::Type : {
        setSettingsBase(settings);
        setSettingsDeviation(settings);
        graphSetData();
        return;
    }  // case zEllipse::Type
    case zPolyline::Type : {
        setSettingsBase(settings);
        setSettingsDeviation(settings);
        graphSetData();
        return;
    }  // case zPolyline::Type
    case zPolygon::Type : {
        setSettingsBase(settings);
        setSettingsDeviation(settings);
        graphSetData();
        return;
    }  // case zPolygon::Type
    }  // switch
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
    emit changeParamsFromDialog(nullptr);
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

void zGraph::calculateProfileWithSigma(QVector<double> x, QVector<double> y, QVector<double> yup, QVector<double> ylw)
{
    profile.clear();
    int d = x.count();
    for (int i=0; i<d; i++) profile.append(QPointF(x[i],y[i]));
    sigma = .0;
    for(int i=0; i<d; i++)
        sigma += (yup[i] - ylw[i]) * (yup[i] - ylw[i]);
    if (d != 0) sigma = qSqrt(sigma / d);
}

int zGraph::calculateProfileWithSnandartDeviation()
{
    int d = imgHand->current_image()->get_bands_count();
    w_len.resize(d);    k_ref.resize(d);  // координаты медианного профиля (исходные)
    keys.resize(d);     values.resize(d);   sigma = .0;

    switch (this->type()) {
    case zPoint::Type : {
        QVector<QPointF> v = imgHand->current_image()->get_profile(this->pos().toPoint());
        if (v.isEmpty()) { qDebug() << "wrong profile size";  return -2; }
        if (v.length() != d) { qDebug() << "wrong profile size";  return -2; }
        foreach(QPointF p, v) {
            int i = v.indexOf(p);
            w_len[i] = v[i].rx();   k_ref[i] = v[i].ry(); }  // foreach
        if (invers_cm)
            foreach(double x, w_len) {
                int i = w_len.count() - 1 - w_len.indexOf(x);
                int j = w_len.indexOf(x);
                keys[j] = 1e7 / w_len[i];     // волновое число см -1
                values[j] = k_ref[i]; }  // foreach
        else
            foreach(double x, w_len) {
                int i = w_len.indexOf(x);
                keys[i] = w_len[i];   values[i] = k_ref[i]; }  // foreach
        // inverse cm
        count_of_point = 1;
        graphSetData();
        buff.clear();
        return 0;
    }  // case zPoint::Type
    case zRect::Type : {
        values_std_dev_upper.resize(d);     values_std_dev_lower.resize(d);
        values.fill(.0);    values_std_dev_upper.fill(INT_MIN);  values_std_dev_lower.fill(INT_MAX);

        calculateMedianProfileForRectEllipseTypes();
        calculateStandardDeviationVectors();  // рассчет стандартного отклонения
        graphSetData();

        buff.clear();
        return 0;
    }  // case zRect::Type
    case zEllipse::Type : {
        values_std_dev_upper.resize(d);     values_std_dev_lower.resize(d);
        values.fill(.0);    values_std_dev_upper.fill(INT_MIN);  values_std_dev_lower.fill(INT_MAX);

        calculateMedianProfileForRectEllipseTypes();
        calculateStandardDeviationVectors();  // рассчет стандартного отклонения
        graphSetData();

        buff.clear();
        return 0;
    }  // case zEllipse::Type
    case zPolyline::Type : {
        values_std_dev_upper.resize(d);     values_std_dev_lower.resize(d);
        values.fill(.0);    values_std_dev_upper.fill(INT_MIN);  values_std_dev_lower.fill(INT_MAX);

        calculateMedianProfileForPolyPolygonTypes();
        calculateStandardDeviationVectors();  // рассчет стандартного отклонения
        graphSetData();

        buff.clear();
        return 0;
    }  // case zPolyline::Type
    case zPolygon::Type : {
        values_std_dev_upper.resize(d);     values_std_dev_lower.resize(d);
        values.fill(.0);    values_std_dev_upper.fill(INT_MIN);  values_std_dev_lower.fill(INT_MAX);

        calculateMedianProfileForPolyPolygonTypes();
        calculateStandardDeviationVectors();  // рассчет стандартного отклонения
        graphSetData();

        buff.clear();
        return 0;
    }  // case zPolygon::Type
    }  // switch

    return -1;
}

void zGraph::setupPlotShowOptions()
{
    plot->setInteraction(QCP::iRangeZoom,true);   // Включаем взаимодействие удаления/приближения
    plot->setInteraction(QCP::iRangeDrag, true);  // Включаем взаимодействие перетаскивания графика
    plot->axisRect()->setRangeDrag(Qt::Vertical);   // перетаскивание только по горизонтальной оси
    plot->axisRect()->setRangeZoom(Qt::Vertical);   // удаления/приближения только по горизонтальной оси
// give the axes some labels:
    if (invers_cm) plot->xAxis->setLabel("волновое число, см-1");
    else plot->xAxis->setLabel("длина волны, нм");
    plot->yAxis->setLabel("коэффициент отражения");
    plot->addGraph();

}

void zGraph::setInversCm(bool b)
{
    invers_cm = b;
}

bool zGraph::getInversCm()
{
    return invers_cm;
}

void zGraph::getSettingsBase(QStringList &strlist, QString zname, int num)
{
    int d = w_len.count() - 1;
    strlist.append(QString("%1%2/invers_cm%3%4").arg(zname).arg(num).arg(setsep).arg(invers_cm));

    QString str("");            // координаты медианного профиля (исходные)
    foreach(double a, w_len) {
        str.append(QString::number(a, 'f', 2));
        if (w_len.indexOf(a) < d) str.append(",");
    }  // for
    strlist.append(QString("%1%2/w_len%3%4").arg(zname).arg(num).arg(setsep).arg(str));
    str = "";            // координаты медианного профиля (исходные)
    foreach(double a, k_ref) {
        str.append(QString::number(a, 'f', 6));
        if (k_ref.indexOf(a) < d) str.append(",");
    }  // for
    strlist.append(QString("%1%2/k_ref%3%4").arg(zname).arg(num).arg(setsep).arg(str));
    str.clear();           // координаты медианного профиля (адаптированные)
    foreach(double a, keys) {
        str.append(QString::number(a, 'f', 2));
        if (keys.indexOf(a) < d) str.append(",");
    }  // for
    strlist.append(QString("%1%2/keys%3%4").arg(zname).arg(num).arg(setsep).arg(str));
    str.clear();            // координаты медианного профиля (адаптированные)
    foreach(double a, values) {
        str.append(QString::number(a, 'f', 6));
        if (values.indexOf(a) < d) str.append(",");
    }  // for
    strlist.append(QString("%1%2/values%3%4").arg(zname).arg(num).arg(setsep).arg(str));
}

void zGraph::getSettingsDeviation(QStringList &strlist, QString zname, int num)
{
    int d = w_len.count() - 1;
    strlist.append(QString("%1%2/sigma%3%4").arg(zname).arg(num).arg(setsep).arg(sigma, 0, 'f', 3));
    strlist.append(QString("%1%2/count_of_point%3%4").arg(zname).arg(num).arg(setsep).arg(count_of_point));
    QString str("");              // координаты стандартного отклонения верх, низ
    foreach(double a, values_std_dev_upper) {
        str.append(QString::number(a, 'f', 6));
        if (values_std_dev_upper.indexOf(a) < d) str.append(",");
    }  // for
    strlist.append(QString("%1%2/values_std_dev_upper%3%4").arg(zname).arg(num).arg(setsep).arg(str));
    str.clear();                  // координаты стандартного отклонения верх, низ
        foreach(double a, values_std_dev_lower) {
            str.append(QString::number(a, 'f', 6));
            if (values_std_dev_lower.indexOf(a) < d) str.append(",");
        }  // for
        strlist.append(QString("%1%2/values_std_dev_lower%3%4").arg(zname).arg(num).arg(setsep).arg(str));
}

void zGraph::setSettingsBase(QSettings *settings)
{
    invers_cm = settings->value("invers_cm").toBool();

    w_len = getVectorFromStr(settings->value("w_len").toString());
    k_ref = getVectorFromStr(settings->value("k_ref").toString());
    keys = getVectorFromStr(settings->value("keys").toString());
    values = getVectorFromStr(settings->value("values").toString());
}

void zGraph::setSettingsDeviation(QSettings *settings)
{
    sigma = settings->value("sigma").toDouble();
    count_of_point = settings->value("count_of_point").toInt();

    values_std_dev_upper = getVectorFromStr(settings->value("values_std_dev_upper").toString());
    values_std_dev_lower = getVectorFromStr(settings->value("values_std_dev_lower").toString());
}

QVector<double> zGraph::getVectorFromStr(QString str)
{
    QStringList strlist = str.split(",");
    QVector<double> v;
    foreach(QString s, strlist) v.append(s.toDouble());
    return v;
}

QString zGraph::getWritableLocation()
{
    QFileInfo info(data_file_name);
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    writableLocation += "/" + info.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    return writableLocation;
}

void zGraph::calculateStandardDeviationVectors(double zfactor)
{
    int d = keys.count();
    sigma = .0;
    if (d <= 1) { // div/0 for std
      for(int i = 0; i < d; i++) {
        values_std_dev_lower[i] = values[i];
        values_std_dev_upper[i] = values[i];
      }
      return;
    }
    QVector<double> std_dev(d, 0.);

    int sigma_count = 0;
    for(int i = 0; i < d; i++) {
      for(int j = 0; j < buff.count(); j++) {
        double a = invers_cm ? buff[j][d - i - 1] - values[i] :
                               buff[j][i]         - values[i]; // value - mean
        std_dev[i] += a * a;
      }
      std_dev[i] = sqrt(std_dev[i] / (d - 1)); // With Bessel's correction
      values_std_dev_lower[i] = values[i] - zfactor * std_dev[i] / sqrt(d);
      values_std_dev_upper[i] = values[i] + zfactor * std_dev[i] / sqrt(d);
      if (values[i] > 1e-5) {
        sigma += std_dev[i] / values[i];
        sigma_count++;
      }
    }
    if (sigma_count > 0) sigma = 100. * sigma / sigma_count;
}

void zGraph::calculateMedianProfileForRectEllipseTypes()
{
    int d = w_len.count();
    buff.clear();

    QRectF rect = this->boundingRect();
// расчет медианного профиля values
    for (int i=rect.left(); i<rect.left()+rect.width()+1; i++)
        for (int j=rect.top(); j<rect.top()+rect.height()+1; j++) {
            if (this->pointIn(QPointF(i,j))) {
                QLineF line = QLineF(0,0,i,j);
                qreal len = line.length();
                qreal a0 = qAtan2(j,i);
                qreal a1 =  qDegreesToRadians(this->rotation());
                qreal a2 = a0 + a1;
                int px = this->pos().x() + len * qCos(a2);
                int py = this->pos().y() + len * qSin(a2);
                QVector<QPointF> v = imgHand->current_image()->get_profile(QPoint(px, py));

                if (v.isEmpty()) { qDebug() << "wrong profile size";  continue; }
                if (v.count() != d) { qDebug() << "wrong profile size";  continue; }

                if (buff.count() == 0) for (int k=0; k<d; k++) w_len[k] = v[k].rx();
                for (int k=0; k<d; k++) k_ref[k] += v[k].ry();
                QVector<double> vy(d);
                for (int k=0; k<d; k++) vy[k] = v[k].ry();
                buff.append(vy);
            }  // if
        }  // for
    count_of_point = buff.count();

    if (buff.count() == 0) {
        for(int i=0; i<d; i++) keys[i] = i;
        values.fill(.0);
        values_std_dev_upper.fill(.0);  values_std_dev_lower.fill(.0);
        return;
    }  // if

    for (int i=0; i<d; ++i) k_ref[i] = k_ref[i] / buff.count();

    if (invers_cm)
        foreach(double x, w_len) {
            int i = w_len.count() - 1 - w_len.indexOf(x);
            int j = w_len.indexOf(x);
            keys[j] = 1e7 / w_len[i];     // волновое число см -1
            values[j] = k_ref[i]; }  // foreach
    else
        foreach(double x, w_len) {
            int i = w_len.indexOf(x);
            keys[i] = w_len[i];   values[i] = k_ref[i]; }  // foreach
    // inverse cm
}

void zGraph::calculateMedianProfileForPolyPolygonTypes()
{
    int d = keys.count();
    buff.clear();
    QRectF rect = this->boundingRect();
// расчет медианного профиля values
    for (int i=rect.left(); i<rect.left()+rect.width()+1; i++)
        for (int j=rect.top(); j<rect.top()+rect.height()+1; j++) {
            if (this->pointIn(QPointF(i,j))) {
                int px = this->pos().x() + i;
                int py = this->pos().y() + j;
                QVector<QPointF> v = imgHand->current_image()->get_profile(QPoint(px, py));

                if (v.isEmpty()) { qDebug() << "wrong profile size";  continue; }
                if (v.count() != d) { qDebug() << "wrong profile size";  continue; }

                if (buff.count() == 0) for (int k=0; k<d; k++) w_len[k] = v[k].rx();
                for (int k=0; k<d; k++) k_ref[k] += v[k].ry();
                QVector<double> vy(d);
                for (int k=0; k<d; k++) vy[k] = v[k].ry();
                buff.append(vy);
            }  // if
        }  // for
    count_of_point = buff.count();
    if (buff.count() == 0) {
        for(int i=0; i<d; i++) keys[i] = i;
        values.fill(.0);
        values_std_dev_upper.fill(.0);  values_std_dev_lower.fill(.0);
        return;
    }  // if

    for (int i=0; i<d; ++i) k_ref[i] = k_ref[i] / buff.count();

    if (invers_cm)
        foreach(double x, w_len) {
            int i = w_len.count() - 1 - w_len.indexOf(x);
            int j = w_len.indexOf(x);
            keys[j] = 1e7 / w_len[i];     // волновое число см -1
            values[j] = k_ref[i]; }  // foreach
    else
        foreach(double x, w_len) {
            int i = w_len.indexOf(x);
            keys[i] = w_len[i];   values[i] = k_ref[i]; }  // foreach
    // inverse cm
}

void zGraph::graphSetData()
{
    dockw->setWindowTitle(this->getTitleExt());
    if (this->type() == zPoint::Type) {
        this->plot->graph(0)->setData(keys, values, true);
        this->plot->xAxis->rescale(true);
        this->plot->replot();
        return;
    }  // if
    this->plot->graph(0)->setData(this->keys, this->values, true);
    this->plot->graph(1)->setData(this->keys, this->values_std_dev_lower, true);
    this->plot->graph(2)->setData(this->keys, this->values_std_dev_upper, true);
    this->plot->xAxis->rescale(true);
    this->plot->replot();
}

void zGraph::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(plot);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *act = menu->addAction("Сохранить изображение профиля ...", this, SLOT(savePlotToPdfJpgPng()));
    act->setIcon(QIcon(":/icons/pdf.png"));
    menu->addSeparator();
    act = menu->addAction("Сохранить профиль в Excel *.csv файл ...", this, SLOT(savePlotToCsv()));
    act->setIcon(QIcon(":/icons/csv2.png"));
    act = menu->addAction("Сохранить область интереса в *.roi файл ...", this, SLOT(savePlotToRoi()));
    act->setIcon(QIcon(":/icons/save.png"));
    menu->addSeparator();
    act = menu->addAction("Масштаб по размеру профиля", this, SLOT(plotRescaleAxes()));
    act->setIcon(QIcon(":/icons/fit-to-height.png"));


    menu->popup(plot->mapToGlobal(pos));
}

void zGraph::savePlotToPdfJpgPng()
{
    QString writableLocation = getWritableLocation();
    QString img_file_name = QFileDialog::getSaveFileName(
        dockw, tr("Сохранить изображение профиля"), writableLocation,
        tr("Файлы PNG (*.png);;Файлы PDF (*.pdf);;Файлы JPG (*.jpg)"));
    if (img_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }
    QFileInfo info(img_file_name);
    if (info.suffix().toLower() == "png") plot->savePng(img_file_name);
    if (info.suffix().toLower() == "pdf") plot->savePdf(img_file_name);
    if (info.suffix().toLower() == "jpg") plot->saveJpg(img_file_name);
}

void zGraph::savePlotToCsv()
{
    QString writableLocation = getWritableLocation();
    QString csv_file_name = QFileDialog::getSaveFileName(
        dockw, tr("Сохранить профиль в Excel *.csv файл"), writableLocation,
        tr("CSV Файлы Excel (*.csv);;CSV Файлы Excel (*.scv)"));
    if (csv_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }
    QFileInfo info(csv_file_name);
    if (info.suffix().toLower() == "csv") {
        QStringList csv_list;
        csv_list.append(data_file_name);
        QString str = "количество точек;";
        str.append(QString("%1;").arg(this->count_of_point));
        csv_list.append(str);
        str = "ср.кв.отклонение, %;";
        str.append(QString("%1;").arg(this->sigma, 0, 'f', 1).replace('.', ','));
        csv_list.append(str);
        str = "длина волны, нм;";
        str.append(this->getTitle());
        csv_list.append(str);

        int count = this->w_len.count();
        for (int i=0; i<count; i++) {
            QString str = QString(tr("%1;").arg(this->keys[i]));
            str.append(QString(tr("%1;").arg(this->values[i])));
            str = str.replace('.', ',');
            csv_list.append(str);
        }  // for
        QFile file(csv_file_name);
        file.remove();
        file.open( QIODevice::Append | QIODevice::Text );
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream.setGenerateByteOrderMark(true);
        foreach(QString str, csv_list) stream << str << endl;
        stream.flush();
        file.close();
    }  // if
}

void zGraph::savePlotToRoi()
{
    QString writableLocation = getWritableLocation();
    QString roi_file_name = QFileDialog::getSaveFileName(
        dockw, tr("Сохранить область интереса в *.roi файл"), writableLocation,
        tr("Файлы областей интереса (*.roi);;Файлы областей интереса (*.roi)"));
    if (roi_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }
    QFileInfo info(roi_file_name);
    if (info.suffix().toLower() == "roi") {
        QSettings settings( roi_file_name, QSettings::IniFormat );
        QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
        settings.setIniCodec( codec );
        settings.clear();
        int num = 0;
        QStringList strlist = this->getSettings(num);
        foreach(QString str, strlist) {
            int index = str.indexOf(this->setsep);
            if (index != -1) {
                QString mid_Key = str.mid(0,index);
                QString mid_Value = str.mid(index+1);
                settings.setValue(mid_Key, mid_Value);
            }  // if
        }  // for
    }  // if
}

void zGraph::plotRescaleAxes()
{
    plot->rescaleAxes();
    plot->yAxis->setRangeLower(.0);
    plot->replot();
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

    strlist.append(QString("Point%1/objectvisible%2%3").arg(num).arg(setsep).arg(this->isVisible()));
    strlist.append(QString("Point%1/title%2%3").arg(num).arg(setsep).arg(getTitle()));
    strlist.append(QString("Point%1/pos%2%3,%4").arg(num).arg(setsep)
                   .arg(pos().rx(),0,'f',2).arg(pos().ry(),0,'f',2));
    strlist.append(QString("Point%1/dockvisible%2%3").arg(num).arg(setsep).arg(dockw->isVisible()));
    QPoint dwpos = dockw->geometry().topLeft();
    strlist.append(QString("Point%1/dockwpos%2%3,%4").arg(num).arg(setsep)
                   .arg(dwpos.x()).arg(dwpos.y()));
// &&& sochi 2020
    getSettingsBase(strlist, "Point", num);

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
    // &&& sochi 2020
    getSettingsBase(strlist, "Polygon", num);
    getSettingsDeviation(strlist, "Polygon", num);

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
    // &&& sochi 2020
    getSettingsBase(strlist, "Rectangle", num);
    getSettingsDeviation(strlist, "Rectangle", num);

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
    // &&& sochi 2020
    getSettingsBase(strlist, "Ellipse", num);
    getSettingsDeviation(strlist, "Ellipse", num);

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
    // &&& sochi 2020
    getSettingsBase(strlist, "Polyline", num);
    getSettingsDeviation(strlist, "Polyline", num);

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

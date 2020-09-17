#include "imagehistogram.h"
#include "addMaskDialog.h"

imageHistogram::imageHistogram(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi();
    move(20,470);  resize(1000,500);

}

//void imageHistogram::updateData(QString data_file_name, QString name, QString formula, QVector<QVector<double> > img,
//                                J09::histogramType &hg)
void imageHistogram::updateData(QString data_file_name, slice_magic *sm)
{
    if (h_data != nullptr) getHistogramFromSliders();

    disconnectSlidersConnections();

    f_title = sm->get_title(); f_formula = sm->get_formula();
    QString plot_title = QString("Гистограмма - [ %1 \\ %2 ]").arg(f_title).arg(f_formula);
    setWindowTitle(plot_title);
    title->setText(plot_title);

    f_data_file_name = data_file_name;

    slice = sm->get_slice();  main_size = QSize(slice[0].count(), slice.count());
    h_data = &sm->h;
    sm_buff = sm;

    updatePreviewData();  // загрузка параметров изображения

//    disconnect();
    setHistogramToSliders();
    updatePreviewImage();  // true - colorized
    previewPlot->rescaleAxes();
    previewPlot->yAxis->setRangeLower(.0);
    previewPlot->yAxis2->setRangeLower(.0);

    graph->setData(h_data->vx, h_data->vy);
    plot->xAxis->setRange(h_data->min, h_data->max);
    plot->yAxis->setRange(0., *std::max_element(h_data->vy.begin(), h_data->vy.end()));
    plot->replot();
    setHistogramToBracked();

    setupSlidersConnections();

    title_of_mask->setText(QString("%1 - %2").arg(f_title).arg(f_formula));
    formula_of_mask->setText(getMaskFormula());

}

void imageHistogram::setPreviewPixmap(QPixmap &mainRGB)
{
    previewRGB = &mainRGB;
}

void imageHistogram::updatePreviewData() {

// Load Options Image as Pixmap
    image_pixmap->topLeft->setCoords(0,main_size.height());
    image_pixmap->bottomRight->setCoords(main_size.width(),0);
    previewPlot->xAxis->setRange(0,main_size.width());
    previewPlot->yAxis->setRange(0,main_size.height());

}

QString imageHistogram::getMaskFormula()
{
    if (h_data->rgb_preview == 3)
        return QString("&%1&[%2-%3]inv").arg(f_formula).arg(h_data->lower).arg(h_data->upper);

    return QString("&%1&[%2-%3]").arg(f_formula).arg(h_data->lower).arg(h_data->upper);
}

QString imageHistogram::getMaskTitle()
{
    return QString("{%1}[%2-%3]").arg(f_title).arg(h_data->lower).arg(h_data->upper);
}

void imageHistogram::calculateHistogram(bool full)
{
    int h = slice.count();  int w = slice.at(0).count();
    if (full) {
        h_data->max = INT_MIN;  h_data->min = INT_MAX;
        for(int y = 0; y < h; y++) {
            h_data->max = std::max(h_data->max, *std::max_element(slice[y].begin(), slice[y].end()));
            h_data->min = std::min(h_data->min, *std::min_element(slice[y].begin(), slice[y].end()));
        }  // for
        if (h_data->max < INT_MIN + 1 || h_data->min > INT_MAX - 1) {
            qDebug() << "slice IS NOT CORRECT!!!";
            return;
        }  // error
        h_data->lower = h_data->min;  h_data->upper = h_data->max;
        h_data->sum = .0;  h_data->sum_of_part = 100.;
        double k = (h_data->hcount-1.)/(h_data->max-h_data->min);
        h_data->vx.resize(h_data->hcount);  h_data->vy.resize(h_data->hcount);  h_data->vy.fill(0.);
        for (int i=0; i<h_data->hcount; i++) h_data->vx[i]=h_data->min+i/k;
        for (int y=0; y<h; y++)
            for (int x=0; x<w; x++){
                int num_int = qRound((slice[y][x]-h_data->min)*k);
                if (num_int < 0) num_int = 0;
                if (num_int > h_data->hcount - 1) num_int = h_data->hcount - 1;
                h_data->vy[num_int]++;
                h_data->sum++;
            }  // for
        return;
    }  // if (full)
    h_data->sum_of_part = 0.;
    for(int i=0;i<h_data->hcount-1;i++) {
        if (h_data->vx[i] < h_data->lower) continue;
        if (h_data->vx[i] > h_data->upper) continue;
        h_data->sum_of_part += h_data->vy[i];
    }  // for
    if (qAbs(h_data->sum) < .000001) {
        h_data->sum_of_part = .0;  return;
    }  // if
    h_data->sum_of_part = 100.*h_data->sum_of_part/h_data->sum;
}

bool imageHistogram::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);

        if( key->key() == Qt::Key_A ) { routate90(false); return true; }  // A
        if( key->key() == Qt::Key_S ) { routate90(true); return true; }  // S
        if( key->key() == Qt::Key_Z ) {
            h_data->rgb_preview = 0; updatePreviewImage(); return true; }  // Z
        if( key->key() == Qt::Key_X ) {
            h_data->rgb_preview = 1; updatePreviewImage(); return true; }  // X
        if( key->key() == Qt::Key_C ) {
            h_data->rgb_preview = 2; updatePreviewImage(); return true; }  // C mask norm
        if( key->key() == Qt::Key_V ) {
            h_data->rgb_preview = 3; updatePreviewImage(); return true; }  // V mask inversion
    }  // KeyPress
    return QDockWidget::eventFilter(object, event);
}

void imageHistogram::setupUi()
{
    setObjectName("histogram");
    setWindowTitle("Гистограмма");
    setAllowedAreas(nullptr);
    setFloating(true);
    installEventFilter(this);

    bottomGroupBox = new QGroupBox("");
    gridLayout = new QGridLayout;

    sliderBrightness = new QSlider(Qt::Horizontal);
    sliderBrightness->setMinimumWidth(150);
    gridLayout->addWidget(sliderBrightness, 1, 0, Qt::AlignLeft);
    sliderLeftColorEdge = new QSlider(Qt::Horizontal);
    sliderLeftColorEdge->setMinimumWidth(150);
    gridLayout->addWidget(sliderLeftColorEdge, 1, 1, Qt::AlignLeft);
    sliderRightColorEdge = new QSlider(Qt::Horizontal);
    sliderRightColorEdge->setMinimumWidth(150);
    gridLayout->addWidget(sliderRightColorEdge, 1, 2, Qt::AlignLeft);
    previewImage = new QCheckBox("Просмотр");
    previewImage->setChecked(true);
    gridLayout->addWidget(previewImage, 2, 0, Qt::AlignLeft);
    labelBrightness = new QLabel("Яркость");
    labelLeftBlue = new QLabel("Цвет минимального индекса");
    labelRightRed = new QLabel("Цвет максимального индекса");
    gridLayout->addWidget(labelBrightness, 0, 0, Qt::AlignLeft);
    gridLayout->addWidget(labelLeftBlue, 0, 1, Qt::AlignLeft);
    gridLayout->addWidget(labelRightRed, 0, 2, Qt::AlignLeft);
    buttonAxisRescale = new QPushButton("Восстановить умолчания");
    buttonAxisRescale->setMinimumHeight(25);
    gridLayout->addWidget(buttonAxisRescale, 2, 1, Qt::AlignLeft);
    buttonMaskSave = new QPushButton("Добавить в список Маски");
    buttonMaskSave->setMinimumHeight(25);
    gridLayout->addWidget(buttonMaskSave, 2, 2, Qt::AlignLeft);

    centralGroupBox = new QGroupBox("Наименование \\ Формула");
    centralGroupBox->setFixedHeight(25);
    centralGBLayout = new QVBoxLayout(centralGroupBox);
    title_of_mask = new QLineEdit("title", centralGroupBox);
    formula_of_mask = new QLineEdit("formula", centralGroupBox);
    centralGBLayout->addWidget(title_of_mask, 1);
    centralGBLayout->addWidget(formula_of_mask, 1);

    bottomGroupBox->setLayout(gridLayout);
    bottomGroupBox->setFixedHeight(75);

    formula_of_mask->setReadOnly(true);
    centralGroupBox->setMaximumHeight(100);

    vertLayout = new QVBoxLayout;
    plot = new QCustomPlot();

    plot->plotLayout()->insertRow(0);
    title = new QCPTextElement(plot, "Гистограмма");
    plot->plotLayout()->addElement(0, 0, title);

    vertLayout->addWidget(plot);
    vertLayout->addWidget(centralGroupBox);
    vertLayout->addWidget(bottomGroupBox);

    labelPreviewStatus->setMaximumHeight(15);
    vertLayout->addWidget(labelPreviewStatus);

//    labelPreview = new QLabel;
    previewPlot = new QCustomPlot();
    setupPreviewPlot();  // настройка параметров просмотра

    horzLayout = new QHBoxLayout;

    horzLayout->addLayout(vertLayout,1);
//    horzLayout->addWidget(labelPreview);
    horzLayout->addWidget(previewPlot,1);

    QWidget *widget  = new QWidget;  // вспомогательный виджет
    widget->setLayout(horzLayout);
    setWidget(widget);
    setMinimumWidth(750);
    setMinimumHeight(300);

    // qcustomplot

    plot->xAxis->setLabel("значение индекса");
    plot->yAxis->setLabel("количество точек");
    graph = plot->addGraph();
    graph->setLineStyle(QCPGraph::lsImpulse);

    // left right - lines   bracket
    // Инициализируем вертикальные линии
    packet_lower = plot->addGraph();
    packet_lower->setPen(QPen(Qt::red));
    packet_upper = plot->addGraph();
    packet_upper->setPen(QPen(Qt::red));
    // add the bracket at the top:
    bracket = new QCPItemBracket(plot);
    bracket->setLength(11.);

    // add the text label at the top:   bracket
    bracketText = new QCPItemText(plot);
    bracketText->position->setParentAnchor(bracket->center);
    bracketText->position->setCoords(0, -bracketTextValue); // move 10 pixels to the top from bracket center anchor
    bracketText->setPositionAlignment(Qt::AlignBottom|Qt::AlignHCenter);
    bracketText->setFont(QFont(font().family(), 8));

    // Инициализируем информационную заливку
    packet_left = plot->addGraph();
    packet_left->setBrush(QBrush(QColor(0, 0, 0, 50)));  // black
    packet_center = plot->addGraph();
    packet_center->setBrush(QBrush(QColor(255, 255, 0, 20)));  // yellow
    packet_right = plot->addGraph();
    packet_right->setBrush(QBrush(QColor(255, 255, 255, 20)));  // white

    setupConnections();
    buttonMaskSave->setEnabled(false);

}

void imageHistogram::setupConnections()
{
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

// Подключаем сигналы событий мыши от полотна графика к слотам для их обработки
    connect(plot, &QCustomPlot::mousePress, this, &imageHistogram::mousePress);
    connect(plot, &QCustomPlot::mouseMove, this, &imageHistogram::mouseMove);
    connect(plot, &QCustomPlot::mouseRelease, this, &imageHistogram::mouseRelease);
// подключаем сигналы и слоты окна
    connect(previewImage,&QCheckBox::stateChanged,this, &imageHistogram::histogramPreview);
    connect(buttonAxisRescale, &QPushButton::clicked, this, &imageHistogram::axisRescale);
    connect(buttonMaskSave, &QPushButton::clicked, this, &imageHistogram::maskSave);
}

void imageHistogram::setupSlidersConnections()
{
// подключаем сигналы и слоты sliders &&&
    connect(sliderBrightness,SIGNAL(valueChanged(int)),this,SLOT(brightnessChanged()));
    connect(sliderLeftColorEdge,SIGNAL(valueChanged(int)),this,SLOT(leftColorChanged()));
    connect(sliderRightColorEdge,SIGNAL(valueChanged(int)),this,SLOT(rightColorChanged()));

}

void imageHistogram::disconnectSlidersConnections()
{
// подключаем сигналы и слоты sliders &&&
    disconnect(sliderBrightness,SIGNAL(valueChanged(int)),this,SLOT(brightnessChanged()));
    disconnect(sliderLeftColorEdge,SIGNAL(valueChanged(int)),this,SLOT(leftColorChanged()));
    disconnect(sliderRightColorEdge,SIGNAL(valueChanged(int)),this,SLOT(rightColorChanged()));
}

QString imageHistogram::getPreviewStatusString()
{
    return previewStatusStr.arg(previewStatusList.at(h_data->rgb_preview));
}

void imageHistogram::brightnessChanged()
{
    getHistogramFromSliders();
    updatePreviewImage();  // true - colorized
}

void imageHistogram::leftColorChanged()
{
    getHistogramFromSliders();
    updatePreviewImage();  // true - colorized
}

void imageHistogram::rightColorChanged()
{
    getHistogramFromSliders();
    updatePreviewImage();  // true - colorized
}

void imageHistogram::setHistogramToSliders()
{
    double d = (h_data->brightness - 1.)*h_data->___sbrightness/.01+h_data->___sbrightness*100.;
    int a = qRound(d);
    sliderBrightness->setValue(a);
    a = qRound((h_data->wl380 - 380.)*100./(781.-380.));
    sliderLeftColorEdge->setValue(a);
    a = qRound((h_data->wl781 - 380.)*100./(781.-380.));
    sliderRightColorEdge->setValue(a);

}

void imageHistogram::getHistogramFromSliders()
{
    h_data->brightness = 1.+(sliderBrightness->value()-h_data->___sbrightness*100.)*.01/h_data->___sbrightness;
    h_data->wl380 = 380. + sliderLeftColorEdge->value() * (781.-380.) / 100.;
    h_data->wl781 = 380. + sliderRightColorEdge->value() * (781.-380.) / 100.;

}

void imageHistogram::updatePreviewImage()
{
    QMatrix rm;    rm.rotate(h_data->rotation);

    switch (h_data->rgb_preview) {
    case 0 :{ QImage img = get_index_rgb_ext( slice, h_data->colorized );
        QPixmap pixmap = changeBrightnessPixmap( img, h_data->brightness );
        pixmap = pixmap.transformed(rm); image_pixmap->setPixmap(pixmap);
        break; }
    case 1 : {
        QPixmap pixmap = previewRGB->transformed(rm); image_pixmap->setPixmap(pixmap); break; }
    case 2 : {
        QImage img = get_mask_image( slice, false );
        QPixmap pixmap = QPixmap::fromImage(img);
        pixmap = pixmap.transformed(rm); image_pixmap->setPixmap(pixmap);
        break; }
    case 3 : {
        QImage img = get_mask_image( slice, true );
        QPixmap pixmap = QPixmap::fromImage(img);
        pixmap = pixmap.transformed(rm); image_pixmap->setPixmap(pixmap);
        break; }
    }  // switch

    previewPlot->replot();

    labelPreviewStatus->setText( getPreviewStatusString() );
    bool enabled = (h_data->rgb_preview == 2 || h_data->rgb_preview == 3);
    buttonMaskSave->setEnabled(enabled);

    formula_of_mask->setText(getMaskFormula());

}

QImage imageHistogram::get_index_rgb_ext(QVector<QVector<double> > &slice, bool colorized)
{
    if(slice.isEmpty()) {
      qDebug() << "CRITICAL! AN EMPTY SLICE RETRIEVED WHILE CONSTRUCTING THE INDEX IMAGE";
      return QImage();
    }
    QSize slice_size(slice[0].count(), slice.count());
    QImage index_img(slice_size, QImage::Format_RGB32);
    double wl_lower = std::min(h_data->wl380,h_data->wl781);
    double wl_upper = std::max(h_data->wl380,h_data->wl781);
    if (qAbs(h_data->upper - h_data->lower) < .000001) {
        index_img.fill(QColor(Qt::black));
        return index_img;
    }  // if
    double k = qAbs(wl_upper - wl_lower) / (h_data->upper - h_data->lower);

    if (colorized) {  // с использованием цветовой схемы
        if (h_data->wl781 > h_data->wl380)
            for(int y = 0; y < slice_size.height(); y++) {
                QRgb *im_scLine = reinterpret_cast<QRgb *>(index_img.scanLine(y));
                for(int x = 0; x < slice_size.width(); x++) {

                    if (slice[y][x] < h_data->lower) {
                        im_scLine[x] = qRgb(0,0,0);  continue;  }  // if
                    if (slice[y][x] > h_data->upper) {
                        im_scLine[x] = qRgb(255,255,255);  continue;  }  // if

                    double virtual_wlen = wl_lower + (slice[y][x] - h_data->lower) * k;

                    im_scLine[x] = get_rainbow_RGB(virtual_wlen);

                }  // for int x
            }  // for int y
        else
            for(int y = 0; y < slice_size.height(); y++) {
                QRgb *im_scLine = reinterpret_cast<QRgb *>(index_img.scanLine(y));
                for(int x = 0; x < slice_size.width(); x++) {

                    if (slice[y][x] < h_data->lower) {
                        im_scLine[x] = qRgb(0,0,0);  continue;  }  // if
                    if (slice[y][x] > h_data->upper) {
                        im_scLine[x] = qRgb(255,255,255);  continue;  }  // if

                    double virtual_wlen = wl_lower + (h_data->upper - slice[y][x]) * k;

                    im_scLine[x] = get_rainbow_RGB(virtual_wlen);

                }  // for int x
            }  // for int y
    } else  // черно-белое изображение
        for(int y = 0; y < slice_size.height(); y++) {
            QRgb *im_scLine = reinterpret_cast<QRgb *>(index_img.scanLine(y));
            for(int x = 0; x < slice_size.width(); x++) {

                    if (slice[y][x] < h_data->lower) {
                        im_scLine[x] = qRgb(0,0,0);  continue;  }  // if
                    if (slice[y][x] > h_data->upper) {
                        im_scLine[x] = qRgb(255,255,255);  continue;  }  // if

                    double d = (slice[y][x] - h_data->lower) / (h_data->upper - h_data->lower) * 255.0;
                    im_scLine[x] =  qRgb(qRound(d), qRound(d), qRound(d));
                }    // for int x
        }  // for int y

    return index_img;
}

QImage imageHistogram::get_mask_image(QVector<QVector<double> > &slice, bool inversion)
{
    if(slice.isEmpty()) {
      qDebug() << "CRITICAL! AN EMPTY SLICE RETRIEVED WHILE CONSTRUCTING THE INDEX IMAGE";
      return QImage();
    }
    QSize slice_size(slice[0].count(), slice.count());
    QImage mask_img(slice_size, QImage::Format_Mono);

    switch (inversion) {
    case true :{
        mask_img.fill(1);
        for(int y = 0; y < slice_size.height(); y++)
            for(int x = 0; x < slice_size.width(); x++) {
                if (slice[y][x] >= h_data->lower && slice[y][x] <= h_data->upper)
                    mask_img.setPixel(x, y, 0);
            }  // for
        break; }
    case false : {
        mask_img.fill(0);
        for(int y = 0; y < slice_size.height(); y++)
            for(int x = 0; x < slice_size.width(); x++) {
                if (slice[y][x] >= h_data->lower && slice[y][x] <= h_data->upper)
                    mask_img.setPixel(x, y, 1);
            }  // for
        break; }
    }  // switch

    return mask_img;

}

QRgb imageHistogram::get_rainbow_RGB(double Wavelength)
{
    static double Gamma = 0.80;
    static double IntensityMax = 255;

    double factor;
    double Red,Green,Blue;

    if ( Wavelength < 380 ) {
        return qRgb(0,0,0);
    }else if((Wavelength >= 380) && (Wavelength<440)){
        Red = -(Wavelength - 440) / (440 - 380);
        Green = 0.0;
        Blue = 1.0;
    }else if((Wavelength >= 440) && (Wavelength<490)){
        Red = 0.0;
        Green = (Wavelength - 440) / (490 - 440);
        Blue = 1.0;
    }else if((Wavelength >= 490) && (Wavelength<510)){
        Red = 0.0;
        Green = 1.0;
        Blue = -(Wavelength - 510) / (510 - 490);
    }else if((Wavelength >= 510) && (Wavelength<580)){
        Red = (Wavelength - 510) / (580 - 510);
        Green = 1.0;
        Blue = 0.0;
    }else if((Wavelength >= 580) && (Wavelength<645)){
        Red = 1.0;
        Green = -(Wavelength - 645) / (645 - 580);
        Blue = 0.0;
    }else if((Wavelength >= 645) && (Wavelength<781)){
        Red = 1.0;
        Green = 0.0;
        Blue = 0.0;
    }else{
        return qRgb(255,255,255);
    }

// Let the intensity fall off near the vision limits

    if((Wavelength >= 380) && (Wavelength<420)){
        factor = 0.3 + 0.7*(Wavelength - 380) / (420 - 380);
    }else if((Wavelength >= 420) && (Wavelength<701)){
        factor = 1.0;
    }else if((Wavelength >= 701) && (Wavelength<781)){
        factor = 0.3 + 0.7*(780 - Wavelength)  / (780 - 700);
    }else{
        factor = 0.0;
    }

    int r = 0, g = 0, b = 0;

// Don't want 0^x = 1 for x <> 0
    const double zero = 0.00000001;
    if (Red < zero) r = 0;
    else r = qRound(IntensityMax * qPow(Red * factor, Gamma));
    r = qMin(255, r);
    if (Green < zero) g = 0;
    else g = qRound(IntensityMax * qPow(Green * factor, Gamma));
    g = qMin(255, g);
    if (Blue < zero) b = 0;
    else b = qRound(IntensityMax * qPow(Blue * factor, Gamma));
    b = qMin(255, b);

    return qRgb(r,g,b);

}

void imageHistogram::setHistogramToBracked()
{
    QCPRange yrange = plot->yAxis->range();
    QVector<double> x(2), y(2);
    x[0] = h_data->lower;  x[1] = x[0];
    y[0] = .0;  y[1] = (bracketTextIndent+.05)*yrange.upper;
    packet_lower->setData(x, y);
    x[0] = h_data->upper;  x[1] = x[0];
    packet_upper->setData(x, y);

    bracketText->setPositionAlignment(Qt::AlignBottom|Qt::AlignHCenter);

    if (h_data->max - h_data->min > 0.00001) {
        double position = (h_data->lower + h_data->upper) / 2.;
        position = (position - h_data->min) / (h_data->max - h_data->min);
        if (position < .1) bracketText->setPositionAlignment(Qt::AlignBottom|Qt::AlignLeft);
        if (position > .9) bracketText->setPositionAlignment(Qt::AlignBottom|Qt::AlignRight);
    }  // if

    bracketText->setText(QString("[%1-%2](%3 %)").arg(h_data->lower,0,'g',3)
                         .arg(h_data->upper,0,'g',3).arg(qRound(h_data->sum_of_part)));
    formula_of_mask->setText(getMaskFormula());

    bracket->left->setCoords(h_data->lower, bracketTextIndent*yrange.upper);
    bracket->right->setCoords(h_data->upper, bracketTextIndent*yrange.upper);

    x[0] = h_data->min;  x[1] = h_data->lower;
    y[0] = 150000.;  y[1] = y[0];
    packet_left->setData(x, y);
    x[0] = h_data->lower;  x[1] = h_data->upper;
    packet_center->setData(x, y);
    x[0] = h_data->upper;  x[1] = h_data->max;
    packet_right->setData(x, y);

    plot->replot();

}

QPixmap imageHistogram::changeBrightnessPixmap(QImage &img, double brightness)
{
    QColor color;
    int h, s, v;
    qreal t_brightnessValue = brightness;
    for(int row = 0; row < img.height(); ++row) {
        uint32_t* data = reinterpret_cast<uint32_t*>(img.scanLine(row));
        for(int col = 0; col < img.width(); ++col) {
            uint32_t & pix = data[col];
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

void imageHistogram::setupPreviewPlot()
{
    previewPlot->resize(500,500);
/// Allow zoom and move
    previewPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
// Add pixmap
    image_pixmap = new QCPItemPixmap(previewPlot);

    image_pixmap->setVisible(true);
    previewPlot->addLayer("image");
    image_pixmap->setLayer("image");
    image_pixmap->setScaled(true);

}

void imageHistogram::mousePress(QMouseEvent *event)
{
    if (histogramPlotTrigger == 0) return;

    double coordX = plot->xAxis->pixelToCoord(event->pos().x());

    switch (histogramPlotTrigger) {
    case 1: {  // left
        histogramPlotPAN = true;
        coordX = qMax(coordX, h_data->min);
        h_data->lower = qMin(coordX, h_data->upper);
        // data
        calculateHistogram(false);
        setHistogramToBracked();
        break;  }
    case 2: {  // right
        histogramPlotPAN = true;
        coordX = qMin(coordX, h_data->max);
        h_data->upper = qMax(coordX, h_data->lower);
        // data
        calculateHistogram(false);
        setHistogramToBracked();
        break;  }
    }  // switch
}

void imageHistogram::mouseMove(QMouseEvent *event)
{
    if (histogramPlotPAN) {
        mousePress(event);
        updatePreviewImage();
        return;
    }  // if

    double coordX = plot->xAxis->pixelToCoord(event->pos().x());
    double dx = h_data->___plotmouse * (h_data->max - h_data->min) / plot->width();

    if ( qAbs(coordX-h_data->lower) < dx ) {
        if (plot->cursor().shape() != Qt::SplitHCursor)
            plot->setCursor(Qt::SplitHCursor);
        histogramPlotTrigger = 1; return; }

    if ( qAbs(coordX-h_data->upper) < dx ) {
        if (plot->cursor().shape() != Qt::SplitHCursor)
            plot->setCursor(Qt::SplitHCursor);
        histogramPlotTrigger = 2; return; }

    if (plot->cursor().shape() != Qt::ArrowCursor)
        plot->setCursor(Qt::ArrowCursor);
    histogramPlotTrigger = 0;

}

void imageHistogram::mouseRelease(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (histogramPlotPAN) {
        histogramPlotPAN = false;  histogramPlotTrigger = 0;
    }  // if
}

void imageHistogram::histogramPreview()
{
    previewPlot->setVisible(previewImage->isChecked());
}

void imageHistogram::axisRescale()
{
    h_data->lower = h_data->min;
    h_data->upper = h_data->max;
    calculateHistogram(false);
    setHistogramToBracked();
    updatePreviewImage();
}

void imageHistogram::maskSave()
{
    mask_appended = new slice_magic();
    mask_appended->set_MASK();
    mask_appended->set_check_state(false);

    mask_appended->set_title(title_of_mask->text());
    mask_appended->set_formula(formula_of_mask->text());
    mask_appended->set_formula_step(QStringList() << mask_appended->get_formula());

    mask_appended->set_inverse(h_data->rgb_preview == 3);

    mask_appended->set_mask(calculateMask( mask_appended->get_inverse() ));
    mask_appended->h.rotation = h_data->rotation;
    mask_appended->set_image(get_mask_image( mask_appended->get_mask() ));
    mask_appended->set_brightness(3.);
    mask_appended->set_rotation(h_data->rotation);

    emit appendMask(mask_appended);

//    QString title(title_of_mask->text());
//    QString formula(formula_of_mask->text());
/*    mask_appended = new J09::maskRecordType;
    mask_appended->checked = true;

    mask_appended->title = title_of_mask->text();
    mask_appended->formula = formula_of_mask->text();
    mask_appended->formula_step_by_step = QStringList() << mask_appended->formula;

    mask_appended->invers = h_data->rgb_preview == 3;

    mask_appended->mask = calculateMask( mask_appended->invers );
    mask_appended->img = get_mask_image(mask_appended->mask);
    mask_appended->brightness = 3.;
    mask_appended->rotation = h_data->rotation;

    emit appendMask(mask_appended); */

}

void imageHistogram::routate90(bool clockwise)
{
    if (clockwise) {
        h_data->rotation += 90.;
        if (h_data->rotation > 360.) h_data->rotation -= 360.;
    } else {
        h_data->rotation -= 90.;
        if (h_data->rotation < 0.) h_data->rotation += 360.;
    }  // if

    updatePreviewImage();

}

void imageHistogram::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(plot);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *act = menu->addAction("Сохранить изображение гистограммы...", this, &imageHistogram::savePlotToPdfJpgPng);
    act->setIcon(QIcon(":/icons/pdf.png"));
    act = menu->addAction("Сохранить гистограмму в Excel CSV файл ...", this, &imageHistogram::saveHistogramToCsv);
    act->setIcon(QIcon(":/icons/csv2.png"));
    menu->addSeparator();
    act = menu->addAction("Сохранить изображение индекса...", this, &imageHistogram::saveIndexToPdfJpgPng);
    act->setIcon(QIcon(":/icons/rainbow_icon_cartoon_style.png"));
    act = menu->addAction("Сохранить изображение маски ...", this, &imageHistogram::saveMaskToPdfJpgPng);
    act->setIcon(QIcon(":/icons/theater.png"));
    act = menu->addAction("Сохранить изображение инвертированной маски ...", this, &imageHistogram::saveInvMaskToPdfJpgPng);
    act->setIcon(QIcon(":/icons/theater.png"));
    act = menu->addAction("Сохранить RGB изображение ...", this, &imageHistogram::saveRGBToPdfJpgPng);
    act->setIcon(QIcon(":/icons/pdf.png"));
    menu->addSeparator();
    act = menu->addAction("Сохранить индекс в Excel CSV файл ...", this, &imageHistogram::saveIndexToCsv);  // -
    act->setIcon(QIcon(":/icons/rainbow_icon_cartoon_style.png"));
    act = menu->addAction("Сохранить маску в Excel CSV файл ...", this, &imageHistogram::saveMaskToCsv);  // -
    act->setIcon(QIcon(":/icons/theater.png"));
    act = menu->addAction("Сохранить инвертированную маску в Excel CSV файл ...", this, &imageHistogram::saveInvMaskToCsv); // -
    act->setIcon(QIcon(":/icons/theater.png"));


    menu->popup(plot->mapToGlobal(pos));

}

QString imageHistogram::getWritableLocation()
{
    QFileInfo info(f_data_file_name);
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    writableLocation += "/" + info.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    return writableLocation;
}

QVector<QVector<int8_t> > imageHistogram::calculateMask(bool inv)
{
    if(slice.isEmpty()) {
      qDebug() << "CRITICAL! AN EMPTY SLICE RETRIEVED WHILE CONSTRUCTING THE INDEX IMAGE";
      return QVector<QVector<int8_t> >();
    }
    QSize slice_size(slice[0].count(), slice.count());
    QVector<QVector<int8_t> > result(slice_size.height(), QVector<int8_t>(slice_size.width()));

    switch (inv) {
    case true :{
        QVector<int8_t> line1(slice_size.width(),1);
        result.fill(line1);
        for(int y = 0; y < slice_size.height(); y++)
            for(int x = 0; x < slice_size.width(); x++) {
                if (slice[y][x] >= h_data->lower && slice[y][x] <= h_data->upper)
                result[y][x] = 0;
            }  // for
        break; }
    case false : {
        QVector<int8_t> line0(slice_size.width(),0);
        result.fill(line0);
        for(int y = 0; y < slice_size.height(); y++)
            for(int x = 0; x < slice_size.width(); x++) {
                if (slice[y][x] >= h_data->lower && slice[y][x] <= h_data->upper)
                    result[y][x] = 1;
            }  // for
        break; }
    }  // switch

    return result;
}

QImage imageHistogram::get_mask_image(QVector<QVector<int8_t> > mask)
{
    QSize slice_size(mask[0].count(), mask.count());
    QImage mask_img(slice_size, QImage::Format_Mono);

        for(int y = 0; y < slice_size.height(); y++)
            for(int x = 0; x < slice_size.width(); x++) {
                mask_img.setPixel(x, y, mask[y][x]);
            }  // for

        return mask_img;
}

void imageHistogram::savePlotToPdfJpgPng()
{
    QString writableLocation = getWritableLocation();
    QString img_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить изображение гистограммы"), writableLocation,
        tr("Файлы PNG (*.png);;Файлы JPG (*.jpg);;Файлы BMP (*.bmp)"));
    if (img_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }
    QFileInfo info(img_file_name);
    if (info.suffix().toLower() == "png") plot->savePng(img_file_name);
    if (info.suffix().toLower() == "pdf") plot->savePdf(img_file_name);
    if (info.suffix().toLower() == "jpg") plot->saveJpg(img_file_name);
}

void imageHistogram::saveHistogramToCsv()
{
    QString writableLocation = getWritableLocation();
    QString csv_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить гистограмму в Excel *.csv файл"), writableLocation,
        tr("Excel файлы (*.csv);;CSV Files (*.scv)"));
    if (csv_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }
    QStringList csv_list;

    csv_list.append("Гистограмма распределения яркостей индексного изображения");
    csv_list.append(f_data_file_name);
    csv_list.append(QString("%1;%2").arg(f_title).arg(f_formula));
    csv_list.append(QString("Количество интервалов (bins);%1").arg(h_data->hcount));
    csv_list.append(QString("Минимальное значение индекса;%1").arg(h_data->min).replace('.', ','));
    csv_list.append(QString("Максимальное значение индекса;%1").arg(h_data->max).replace('.', ','));
    csv_list.append(QString("значение интервала;количество точек в интервале"));
    for(int i=0; i<h_data->vx.count(); i++)
        csv_list.append(QString("%1;%2").arg(h_data->vx[i]).arg(h_data->vy[i]).replace('.', ','));

    QFile file(csv_file_name);
    file.remove();
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(true);
    foreach(QString str, csv_list) stream << str << endl;
    stream.flush();
    file.close();
}

void imageHistogram::saveIndexToPdfJpgPng()
{
    QMatrix rm;    rm.rotate(h_data->rotation);
    QImage img = get_index_rgb_ext( slice, h_data->colorized );
    QPixmap pixmap = changeBrightnessPixmap( img, h_data->brightness );
    pixmap = pixmap.transformed(rm);

    QString writableLocation = getWritableLocation();
    QString img_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить изображение индекса"), writableLocation,
        tr("Файлы PNG (*.png);;Файлы JPG (*.jpg);;Файлы JPEG (*.jpeg);;Файлы BMP (*.bmp)"));
    if (img_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }

    QFileInfo info(img_file_name);
    if (info.suffix().toLower() == "png") pixmap.save(img_file_name, "PNG");
    if (info.suffix().toLower() == "jpg") pixmap.save(img_file_name, "JPG");
    if (info.suffix().toLower() == "jpeg") pixmap.save(img_file_name, "JPEG");
    if (info.suffix().toLower() == "bmp") pixmap.save(img_file_name, "BMP");

}

void imageHistogram::saveMaskToPdfJpgPng()
{
    QMatrix rm;    rm.rotate(h_data->rotation);
    QImage img = get_mask_image( slice, false );
    img = img.transformed(rm);

    QString writableLocation = getWritableLocation();
    QString img_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить изображение маски"), writableLocation,
        tr("Файлы PNG (*.png);;Файлы JPG (*.jpg);;Файлы BMP (*.bmp)"));
    if (img_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }

    QFileInfo info(img_file_name);
    if (info.suffix().toLower() == "png") img.save(img_file_name, "PNG");
    if (info.suffix().toLower() == "jpg") img.save(img_file_name, "JPG");
    if (info.suffix().toLower() == "bmp") img.save(img_file_name, "BMP");

}

void imageHistogram::saveInvMaskToPdfJpgPng()
{
    QMatrix rm;    rm.rotate(h_data->rotation);
    QImage img = get_mask_image( slice, true );
    img = img.transformed(rm);

    QString writableLocation = getWritableLocation();
    QString img_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить изображение инвертированной маски"), writableLocation,
        tr("Файлы PNG (*.png);;Файлы JPG (*.jpg);;Файлы BMP (*.bmp)"));
    if (img_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }

    QFileInfo info(img_file_name);
    if (info.suffix().toLower() == "png") img.save(img_file_name, "PNG");
    if (info.suffix().toLower() == "jpg") img.save(img_file_name, "JPG");
    if (info.suffix().toLower() == "bmp") img.save(img_file_name, "BMP");

}

void imageHistogram::saveRGBToPdfJpgPng()
{
    QMatrix rm;    rm.rotate(h_data->rotation);
    QPixmap pixmap = previewRGB->transformed(rm);

    QString writableLocation = getWritableLocation();
    QString img_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить RGB изображение"), writableLocation,
        tr("Файлы PNG (*.png);;Файлы JPG (*.jpg);;Файлы JPEG (*.jpeg);;Файлы BMP (*.bmp)"));
    if (img_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }

    QFileInfo info(img_file_name);
    if (info.suffix().toLower() == "png") pixmap.save(img_file_name, "PNG");
    if (info.suffix().toLower() == "jpg") pixmap.save(img_file_name, "JPG");
    if (info.suffix().toLower() == "bmp") pixmap.save(img_file_name, "BMP");
    if (info.suffix().toLower() == "jpeg") pixmap.save(img_file_name, "JPEG");

}

void imageHistogram::saveIndexToCsv()
{
    QString writableLocation = getWritableLocation();
    QString csv_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить индекс в Excel CSV файл"), writableLocation,
        tr("Excel CSV файлы (*.csv)"));
    if (csv_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }
    sm_buff->set_title(title_of_mask->text());
    sm_buff->set_formula(formula_of_mask->text());
    sm_buff->save_index_to_CSV_file(csv_file_name);

}

void imageHistogram::saveMaskToCsv()
{
    QString writableLocation = getWritableLocation();
    QString csv_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить маску в Excel CSV файл"), writableLocation,
        tr("Excel CSV файлы (*.csv)"));
    if (csv_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }

    sm_buff->set_title(title_of_mask->text());
    sm_buff->set_formula(formula_of_mask->text());
    sm_buff->save_mask_to_CSV_file_from_slice(csv_file_name, false);

}

void imageHistogram::saveInvMaskToCsv()
{
    QString writableLocation = getWritableLocation();
    QString csv_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить инвертированную маску в Excel CSV файл"), writableLocation,
        tr("Excel CSV файлы (*.csv)"));
    if (csv_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }

    sm_buff->set_title(title_of_mask->text());
    QString f = formula_of_mask->text();
    if (f.indexOf("inv") == -1) f.append("inv");
    sm_buff->set_formula(f);
    sm_buff->save_mask_to_CSV_file_from_slice(csv_file_name, true);

}

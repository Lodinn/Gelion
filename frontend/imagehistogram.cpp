#include "imagehistogram.h"

imageHistogram::imageHistogram(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi();
    move(20,470);  resize(1000,500);

}

void imageHistogram::updateData(QString name, QString formula, QVector<QVector<double> > img,
                                J09::histogramType &hg)
{
    if (h_data != nullptr) getHistogramFromSliders();

    this->disconnect();
    setWindowTitle(QString("Гистограмма - [ %1 \\ %2 ]").arg(name).arg(formula));

    slice = img;  main_size = QSize(slice[0].count(), slice.count());
    h_data = &hg;

    updatePreviewData();  // загрузка параметров изображения

    disconnect();
    setHistogramToSliders();
    updatePreviewImage();  // true - colorized
    previewPlot->rescaleAxes();

    graph->setData(h_data->vx, h_data->vy);
    plot->xAxis->setRange(h_data->min, h_data->max);
    plot->yAxis->setRange(0., *std::max_element(h_data->vy.begin(), h_data->vy.end()));
    plot->replot();
    setHistogramToBracked();

    setupConnections();

}

void imageHistogram::setPreviewPixmap(QPixmap &mainRGB)
{
    previewRGB = &mainRGB;
}

void imageHistogram::updatePreviewData() {

// Load Options Image as Pixmap
    image_pixmap->topLeft->setCoords(0,0);
    image_pixmap->bottomRight->setCoords(main_size.width(),main_size.height());
    previewPlot->xAxis->setRange(0,main_size.width());
    previewPlot->yAxis->setRange(0,main_size.height());

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

        if(key->key() == Qt::Key_W) {

            if (h_data->rgb_preview == 0) {
                h_data->rgb_preview = 1;
                updatePreviewImage();  // true - colorized
                return true;
            }
            if (h_data->rgb_preview == 1) {
                h_data->rgb_preview = 0;
                updatePreviewImage();  // true - colorized
                return true;
            }

        }
    }
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
    gridLayout->addWidget(buttonAxisRescale, 2, 1, Qt::AlignLeft);
    button90routate = new QPushButton("Повернуть на 90 градусов");
    gridLayout->addWidget(button90routate, 2, 2, Qt::AlignLeft);

    bottomGroupBox->setLayout(gridLayout);
    bottomGroupBox->setMaximumHeight(100);

    vertLayout = new QVBoxLayout;
    plot = new QCustomPlot();

    vertLayout->addWidget(plot);
    vertLayout->addWidget(bottomGroupBox);

//    labelPreview = new QLabel;
    previewPlot = new QCustomPlot();
    setupPreviewPlot();  // настройка параметров просмотра

    horzLayout = new QHBoxLayout;

    horzLayout->addLayout(vertLayout);
//    horzLayout->addWidget(labelPreview);
    horzLayout->addWidget(previewPlot);

//    horzLayout->setStretch(0,1);

    QWidget *widget  = new QWidget;  // вспомогательный виджет
    widget->setLayout(horzLayout);
    setWidget(widget);
    setMinimumWidth(750);
    setMinimumHeight(300);
//    setMaximumHeight(530);

    // data
    // ........

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

}

void imageHistogram::setupConnections()
{

    // Подключаем сигналы событий мыши от полотна графика к слотам для их обработки
    connect(plot, &QCustomPlot::mousePress, this, &imageHistogram::mousePress);
    connect(plot, &QCustomPlot::mouseMove, this, &imageHistogram::mouseMove);
    connect(plot, &QCustomPlot::mouseRelease, this, &imageHistogram::mouseRelease);
    // подключаем сигналы и слоты окна
    connect(previewImage,&QCheckBox::stateChanged,this, &imageHistogram::histogramPreview);
    connect(buttonAxisRescale, &QPushButton::clicked, this, &imageHistogram::axisRescale);
    connect(button90routate, &QPushButton::clicked, this, &imageHistogram::routate90);
    connect(sliderBrightness,SIGNAL(valueChanged(int)),this,SLOT(brightnessChanged()));
    connect(sliderLeftColorEdge,SIGNAL(valueChanged(int)),this,SLOT(leftColorChanged()));
    connect(sliderRightColorEdge,SIGNAL(valueChanged(int)),this,SLOT(rightColorChanged()));

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
//    labelPreview->setPixmap(pixmap);
    QMatrix rm;    rm.rotate(h_data->rotation);

    switch (h_data->rgb_preview) {
    case 0 :{ QImage img = get_index_rgb_ext( slice, h_data->colorized );
        QPixmap pixmap = changeBrightnessPixmap( img, h_data->brightness );
        pixmap = pixmap.transformed(rm); image_pixmap->setPixmap(pixmap);
        break; }
    case 1 : {
        QPixmap pixmap = previewRGB->transformed(rm); image_pixmap->setPixmap(pixmap); break; }
    }  // switch

    image_pixmap->topLeft->setCoords(0,0);
    image_pixmap->bottomRight->setCoords(main_size.width(),main_size.height());
    previewPlot->replot();
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
    bracket->left->setCoords(h_data->lower, bracketTextIndent*yrange.upper);
    bracket->right->setCoords(h_data->upper, bracketTextIndent*yrange.upper);

    x[0] = h_data->min;  x[1] = h_data->lower;
    y[0] = 50000.;  y[1] = y[0];
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
        updatePreviewImage();  // true - colorized
        return;
    }  // if

    double coordX = plot->xAxis->pixelToCoord(event->pos().x());

    if (qAbs(coordX-h_data->lower)<h_data->___plotmouse) {
        if (plot->cursor().shape() != Qt::SplitHCursor)
            plot->setCursor(Qt::SplitHCursor);
        histogramPlotTrigger = 1; return; }

    if (qAbs(coordX-h_data->upper)<h_data->___plotmouse) {
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
//    labelPreview->setVisible(previewImage->isChecked());
    previewPlot->setVisible(previewImage->isChecked());
}

void imageHistogram::axisRescale()
{
    h_data->lower = h_data->min;
    h_data->upper = h_data->max;
    calculateHistogram(false);
    setHistogramToBracked();
    updatePreviewImage();  // true - colorized
}

void imageHistogram::routate90()
{
    h_data->rotation += 90.;
    if (h_data->rotation > 360.) h_data->rotation -= 360.;
    updatePreviewImage();  // true - colorized
    qDebug() << "h_data->rotation" << h_data->rotation;
}

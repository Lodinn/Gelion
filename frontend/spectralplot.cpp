#include "spectralplot.h"

SpectralPlot::SpectralPlot(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi();
    move(20,20);  resize(1000,600);
    setupConnections();

// цвета и стили профилей
    J09::plotStyle sct;
    sct.style = 1;
    sct.color = QColor(31, 119, 180); plot_styles_cycle.append(sct);
    sct.color = QColor(255, 127, 14); plot_styles_cycle.append(sct);
    sct.color = QColor(44, 160, 44); plot_styles_cycle.append(sct);
    sct.color = QColor(214, 39, 40); plot_styles_cycle.append(sct);
    sct.color = QColor(148, 103, 189); plot_styles_cycle.append(sct);
    sct.color = QColor(140, 86, 75); plot_styles_cycle.append(sct);
    sct.color = QColor(227, 119, 194); plot_styles_cycle.append(sct);
    sct.color = QColor(127, 127, 127); plot_styles_cycle.append(sct);
    sct.color = QColor(188, 189, 34); plot_styles_cycle.append(sct);
    sct.color = QColor(23, 190, 207); plot_styles_cycle.append(sct);
//    sct.color = QColor(255,0,0);  sct.style = 1; plot_styles_cycle.append(sct); // red style 1
//    sct.color = QColor(0,255,0);  sct.style = 2; plot_styles_cycle.append(sct); // green style 2
//    sct.color = QColor(0,0,255);  sct.style = 3; plot_styles_cycle.append(sct); // blue style 3
//    sct.color = QColor(57,57,57);  sct.style = 4; plot_styles_cycle.append(sct); // grey style 4

//    sct.color = QColor(255,0,0);  sct.style = 5; plot_styles_cycle.append(sct); // red style 5
//    sct.color = QColor(0,255,0);  sct.style = 6; plot_styles_cycle.append(sct); // green style 6
//    sct.color = QColor(0,0,255);  sct.style = 7; plot_styles_cycle.append(sct); // blue style 7
//    sct.color = QColor(57,57,57);  sct.style = 8; plot_styles_cycle.append(sct); // grey style 8

//    sct.color = QColor(255,0,0);  sct.style = 4; plot_styles_cycle.append(sct); // red style 4
//    sct.color = QColor(0,255,0);  sct.style = 3; plot_styles_cycle.append(sct); // green style 3
//    sct.color = QColor(0,0,255);  sct.style = 2; plot_styles_cycle.append(sct); // blue style 2
//    sct.color = QColor(57,57,57);  sct.style = 1; plot_styles_cycle.append(sct); // grey style 1

//    sct.color = QColor(255,0,0);  sct.style = 8; plot_styles_cycle.append(sct); // red style 8
//    sct.color = QColor(0,255,0);  sct.style = 7; plot_styles_cycle.append(sct); // green style 7
//    sct.color = QColor(0,0,255);  sct.style = 6; plot_styles_cycle.append(sct); // blue style 6
//    sct.color = QColor(57,57,57);  sct.style = 5; plot_styles_cycle.append(sct); // grey style 5

//    sct.color = QColor(255,0,0);  sct.style = 2; plot_styles_cycle.append(sct); // red style 8
//    sct.color = QColor(0,255,0);  sct.style = 4; plot_styles_cycle.append(sct); // green style 7
//    sct.color = QColor(0,0,255);  sct.style = 1; plot_styles_cycle.append(sct); // blue style 6
//    sct.color = QColor(57,57,57);  sct.style = 3; plot_styles_cycle.append(sct); // grey style 5
}

void SpectralPlot::updateData(QString data_file_name, QList<zGraph *> list, bool rescale)
{
    f_data_file_name = data_file_name;
    graph_list = list;
    if (list.count() == 0) return;
    zGraph *item = list[0];
    if (item->getInversCm())
        plot->xAxis->setLabel(QString("волновое число, см-1 ( %1 )").arg("v"));
    else
        plot->xAxis->setLabel(QString("длина волны, нм ( %1 )").arg(QString(u8"\u03BB")));

    // data
    plot->clearGraphs();        createRainbow();       updateRainbow();
    plot->xAxis->setRange(390, 1010); plot->yAxis->setRange(0, 1.5);
    return;

    /*sXmin = INT_MAX;  sXmax = INT_MIN;
    sYmin = .0;  sYmax = INT_MIN;
    foreach(zGraph *item, list) {
        if (!item->isVisible()) continue;
        sXmin = std::min(sXmin, *std::min_element(item->keys.begin(), item->keys.end()));
        sXmax = std::max(sXmax, *std::max_element(item->keys.begin(), item->keys.end()));
        sYmax = std::max(sYmax, *std::max_element(item->values.begin(), item->values.end()));
        sYmax = std::max(sYmax, *std::max_element(item->values_std_dev_upper.begin(), item->values_std_dev_upper.end()));
        plot->addGraph();
        plot->graph()->setName(item->getTitle());
        plot->graph()->setData(item->keys, item->values);
        QPen graphPen;

        int num = list.indexOf(item) % plot_styles_cycle.count();
        graphPen.setColor(plot_styles_cycle[num].color);
        graphPen.setWidthF(1.);
        plot->graph()->setPen(graphPen);

        int style = plot_styles_cycle[num].style;
        plot->graph()->setScatterStyle(QCPScatterStyle(static_cast<QCPScatterStyle::ScatterShape>(style)));
    }  // foreach

    if (rainbowCheckBox->isChecked()) rainbowTransparency = 50;
    else rainbowTransparency = 0;
    drawRainbowSpectralRanges();
    spectralSetAllRange();          */

//    if (rescale) spectralSetAllRange();

//    // spectral colors
//    int count = getGraphCount();
//    if (rainbowCheckBox->isChecked()) setRainbowSpectralRanges();
//    mRainbow = getGraphCount() - count;

//    setStdVevProfiles();

    plot->replot();

}

void SpectralPlot::updateDataOneProfile(zGraph *item, int num)
{
    if (num > plot->graphCount() - 1) return;
    if (item->getInversCm())
        plot->xAxis->setLabel(QString("волновое число, см-1 ( %1 )").arg("v"));
    else
        plot->xAxis->setLabel(QString("длина волны, нм ( %1 )").arg(QString(u8"\u03BB")));

// &&& sochi 2020
    plot->graph(num)->setData(item->keys, item->values);

    qDebug() << "item->graph_lower" << item->values_std_dev_lower;
    qDebug() << "item->graph_upper" << item->values_std_dev_upper;

//    item->graph_lower->setData(item->keys, item->values_std_dev_lower);
//    item->graph_upper->setData(item->keys, item->values_std_dev_upper);

    plot->replot();
}

void SpectralPlot::setRainbowSpectralRanges() {
    // attempt to nice picture

    int legeng_min = plot->legend->itemCount();

    QVector<double> x(2), y(2, 50);

    x[0] = 700.;  x[1] = 635.;
    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(255, 0, 0, 50)));
    x[0] = 560.;  x[1] = 520.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(0, 255, 0, 50)));
    // blue
    x[0] = 490.;  x[1] = 450.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(0, 0, 255, 50)));
    // violet
    x[0] = 450.;  x[1] = 380.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(255, 0, 255, 50)));

    x[0] = 590.;  x[1] = 560.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(255, 255, 0, 50)));
    x[0] = 520.;  x[1] = 490.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(0, 255, 255, 50)));
    x[0] = 635.;  x[1] = 590.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(255, 128, 0, 50)));

    int legeng_max = plot->legend->itemCount();

    for(int i = legeng_max - 1; i >= legeng_min; i--) {
//        QCPAbstractLegendItem *item = spectralAnalisysPlot->legend->item(i);
//        item->setVisible(false);
        plot->legend->removeItem(i);
    }  // for

}

bool SpectralPlot::eventFilter(QObject *object, QEvent *event)
{
    if (object == plot &&
            event->type() == QEvent::KeyPress) {

        QKeyEvent *key = static_cast<QKeyEvent *>(event);

        if(key->key() == Qt::Key_Left && tracer->style() == QCPItemTracer::tsCrosshair) {

            QCPGraph *graph = tracer->graph();
            if (tracerIndex > 0) {
                tracerIndex--;
                double value = graph->data()->at(tracerIndex)->mainKey();
                tracer->setGraphKey(value);
                plot->replot();

                double dataValue = graph->data()->at(tracerIndex)->mainValue();
                auto message = getGraphClickedStrings(tracerIndex, graph->name(), value, dataValue);
                textEdit->insertPlainText(message.at(0));
                statusBar->showMessage(message.at(1));

            }  // if
            return true;
        }  // if Key_Left
        if(key->key() == Qt::Key_Right && tracer->style() == QCPItemTracer::tsCrosshair) {

            QCPGraph *graph = tracer->graph();
            if (tracerIndex < graph->dataCount() - 1) {
                tracerIndex++;
                double value = graph->data()->at(tracerIndex)->mainKey();
                tracer->setGraphKey(value);
                plot->replot();

                double dataValue = graph->data()->at(tracerIndex)->mainValue();
                auto message = getGraphClickedStrings(tracerIndex, graph->name(), value, dataValue);
                textEdit->insertPlainText(message.at(0));
                statusBar->showMessage(message.at(1));

            }  // if
            return true;
        }  // if Key_Right
    }  // if QEvent::KeyPress

    return QDockWidget::eventFilter(object, event);
}

void SpectralPlot::setupUi()
{
    setObjectName("imageSpectral");
    setWindowTitle("Спектральный анализ");
    setAllowedAreas(nullptr);  setFloating(true);

    installEventFilter(this);
    setupRainbowCoordinates();

    resize(900,600);  move(500,50);
    bottomGroupBox = new QGroupBox("");
    gridLayout = new QGridLayout;
    textEdit = new QTextEdit;
    textEdit->resize(500,100);
    textEdit->setMinimumWidth(400);
    gridLayout->addWidget(textEdit, 0, 0, Qt::AlignLeft);
    axisResizeButton = new QPushButton("Полный масштаб");
    gridLayout->addWidget(axisResizeButton, 0, 1, Qt::AlignLeft);
    rainbowCheckBox = new QCheckBox("Заливка спектральных диапазонов");
    rainbowCheckBox->setChecked(true);
    gridLayout->addWidget(rainbowCheckBox, 1, 0, Qt::AlignLeft);

    statusBar = new QStatusBar;  //in constructor for example
    statusBar->showMessage(tr("Здесь появится информация о точке выбранного профиля"));
    statusBar->setMaximumWidth(500);
    gridLayout->addWidget(statusBar, 1, 1, Qt::AlignBottom);

    bottomGroupBox->setLayout(gridLayout);
    bottomGroupBox->setMaximumHeight(120);

    dockLayout = new QVBoxLayout;
    plot = new QCustomPlot();
    plot->setCursor(Qt::CrossCursor);

    plot->installEventFilter(this);

    dockLayout->addWidget(plot);
    dockLayout->addWidget(bottomGroupBox);

    QWidget *widget  = new QWidget;  // вспомогательный виджет
    widget->setLayout(dockLayout);
    setWidget(widget);

    // qcustomplot

    // Инициализируем трассировщик большой и маленький
    tracer = new QCPItemTracer(plot);
    tracer_tolltip = new QCPItemTracer(plot);
    tracer_tolltip->setInterpolating(false);
    tracer_tolltip->setPen(QPen(Qt::red));
    tracer_tolltip->setBrush(Qt::transparent);
    tracer_tolltip->setSize(9);

    // Interactions
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    plot->axisRect()->setupFullAxesBox();
    plot->plotLayout()->insertRow(0);

    QCPTextElement *title = new QCPTextElement(plot,
                    "Спектральные профили областей интереса");
    plot->plotLayout()->addElement(0, 0, title);
    plot->xAxis->setLabel(QString("длина волны, нм ( %1 )").arg(QString(u8"\u03BB")));
    plot->yAxis->setLabel("коэффициент отражения ( r )");
    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignTop);
    QFont legendFont = QFont("Helvetica", 8);
    plot->legend->setFont(legendFont);
    plot->legend->setSelectedFont(legendFont);
    plot->legend->setSelectableParts(QCPLegend::spItems);

}

void SpectralPlot::setupConnections()
{
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    // allRange
    connect(axisResizeButton, &QPushButton::clicked, this, &SpectralPlot::spectralSetAllRange);

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(plot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    connect(plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    connect(plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));

    // make bottom and left axes transfer their ranges to top and right axes:
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));

    // connect slot that shows a message in the status bar when a graph is clicked:
    connect(plot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));

    // connect rainbowCheckBox
    connect(rainbowCheckBox, &QCheckBox::clicked, this, &SpectralPlot::updateRainbow);
}

void SpectralPlot::rainbowShow(bool checked)
{
   if (checked) rainbowTransparency = 50;
    else rainbowTransparency = 0;
    updateRainbowSpectralRanges();
    plot->replot();
}

void SpectralPlot::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(plot);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *act = menu->addAction("Сохранить изображение спектральных профилей...", this, &SpectralPlot::savePlotToPdfJpgPng);
    act->setIcon(QIcon(":/icons/pdf.png"));
    menu->addSeparator();
    act = menu->addAction("Сохранить спектральные профили в Excel *.csv файл ...", this, &SpectralPlot::savePlotToCsv);
    act->setIcon(QIcon(":/icons/csv2.png"));
    act = menu->addAction("Сохранить области интереса в *.roi файл ...", this, &SpectralPlot::savePlotToRoi);
    act->setIcon(QIcon(":/icons/save.png"));

    menu->popup(plot->mapToGlobal(pos));

}

void SpectralPlot::savePlotToPdfJpgPng()
{
    QString writableLocation = getWritableLocation();
    QString img_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить изображение спектральных профилей"), writableLocation,
        tr("Файлы PNG (*.png);;Файлы PDF (*.pdf);;Файлы JPG (*.jpg)"));
    if (img_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }
    QFileInfo info(img_file_name);
    if (info.suffix().toLower() == "png") plot->savePng(img_file_name);
    if (info.suffix().toLower() == "pdf") plot->savePdf(img_file_name);
    if (info.suffix().toLower() == "jpg") plot->saveJpg(img_file_name);
}

void SpectralPlot::savePlotToCsv()
{
    if (graph_list.count() == 0) return;
    QString writableLocation = getWritableLocation();
    QString csv_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить спектральные профили в Excel *.csv файл"), writableLocation,
        tr("CSV Файлы Excel (*.csv);;CSV Файлы Excel (*.scv)"));
    if (csv_file_name.isEmpty()) {
        qDebug() << "wrong file name";
        return; }
    QFileInfo info(csv_file_name);
    if (info.suffix().toLower() == "csv") {
        QStringList csv_list;
        csv_list.append(f_data_file_name);
        QString str = "количество точек;";
        foreach(zGraph *item, graph_list)
            if (item->isVisible()) str.append(QString("%1;").arg(item->count_of_point));
        csv_list.append(str);
        str = "ср.кв.отклонение, %;";
        foreach(zGraph *item, graph_list)
            if (item->isVisible()) str.append(QString("%1;").arg(item->sigma, 0, 'f', 1).replace('.', ','));
        csv_list.append(str);
        str = "длина волны, нм;";
        foreach(zGraph *item, graph_list)
            if (item->isVisible()) str.append(item->getTitle() + ";");
        csv_list.append(str);

        int count = graph_list[0]->w_len.count();
        for (int i=0; i<count; i++) {
            QString str = QString(tr("%1;").arg(graph_list[0]->keys[i]));
            foreach(zGraph *item, graph_list)
                if (item->isVisible()) str.append(QString(tr("%1;").arg(item->values[i])));
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

void SpectralPlot::savePlotToRoi()
{
    if (graph_list.count() == 0) return;
    QString writableLocation = getWritableLocation();
    QString roi_file_name = QFileDialog::getSaveFileName(
        this, tr("Сохранить области интереса в *.roi файл"), writableLocation,
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
        foreach(zGraph *item, graph_list) {
            if (!item->isVisible()) continue;
            QStringList strlist = item->getSettings(num);
            foreach(QString str, strlist) {
                int index = str.indexOf(item->setsep);
                if (index == -1) continue;
                QString mid_Key = str.mid(0,index);
                QString mid_Value = str.mid(index+1);
                settings.setValue(mid_Key, mid_Value);
            }  // for
            num++;
        }  // for
    }  // if
}

void SpectralPlot::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
    // since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
    // usually it's better to first check whether interface1D() returns non-zero, and only then use it.
    double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
    double keyValue = plottable->interface1D()->dataMainKey(dataIndex);
    auto message = getGraphClickedStrings(dataIndex, plottable->name(), keyValue, dataValue);
    textEdit->insertPlainText(message.at(0));
    if (plot->selectedGraphs().count() == 1) {
        tracerIndex = dataIndex;
        QCPGraph *graph = plot->selectedGraphs().at(0);
        tracer->setStyle(QCPItemTracer::tsCrosshair);
        tracer->setGraph(graph);
        tracer->setGraphKey(keyValue);
        statusBar->showMessage(message.at(1));
    }  // if
}

QStringList SpectralPlot::getGraphClickedStrings(int dataIndex, QString name, double keyValue, double dataValue) {
    QStringList strlist;
    strlist.append(QString(u8"\u03BB = %1 \u043D\u043C r = %2\n").arg(keyValue).arg(dataValue));
    // λ = 807.84 нм r = 0.102217
    strlist.append(QString("Спектр '%1' точка #%2 ( дл.волны %3 нм ) коэффициент отражения %4").
                   arg(name).arg(dataIndex).arg(keyValue).arg(dataValue));
    return strlist;
}

void SpectralPlot::updateDataRainbow(bool r_show)
{
    // data
    plot->clearGraphs();
    foreach(zGraph *item, graph_list) {
        if (!item->isVisible()) continue;
        int n = item->profile.count();
        QVector<double> x(n), y(n);
        for (int i=0; i<n; i++) { x[i] = item->profile[i].rx(); y[i] = item->profile[i].ry(); }
        plot->addGraph();
        plot->graph()->setName(item->getTitle());
        plot->graph()->setData(x, y);
         QPen graphPen;

        int num = graph_list.indexOf(item) % plot_styles_cycle.count();
        graphPen.setColor(plot_styles_cycle[num].color);
        graphPen.setWidthF(1.);
        plot->graph()->setPen(graphPen);

        int style = plot_styles_cycle[num].style;
        plot->graph()->setScatterStyle(QCPScatterStyle(static_cast<QCPScatterStyle::ScatterShape>(style)));
    }  // foreach

    // spectral colors
    int count = getGraphCount();
    if (r_show) setRainbowSpectralRanges();
    mRainbow = getGraphCount() - count;

    plot->replot();
}

void SpectralPlot::drawRainbowSpectralRanges()
{
    rainbowNum = plot->legend->itemCount();

    QVector<double> x(2), y(2, 50);

    x[0] = 700.;  x[1] = 635.;
    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(255, 0, 0, rainbowTransparency)));
    x[0] = 560.;  x[1] = 520.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(0, 255, 0, rainbowTransparency)));
    // blue
    x[0] = 490.;  x[1] = 450.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(0, 0, 255, rainbowTransparency)));
    // violet
    x[0] = 450.;  x[1] = 380.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(255, 0, 255, rainbowTransparency)));

    x[0] = 590.;  x[1] = 560.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(255, 255, 0, rainbowTransparency)));
    x[0] = 520.;  x[1] = 490.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(0, 255, 255, rainbowTransparency)));
    x[0] = 635.;  x[1] = 590.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(255, 128, 0, rainbowTransparency)));

    int legeng_max = plot->legend->itemCount();

    for(int i = legeng_max - 1; i >= rainbowNum; i--) {
        plot->legend->removeItem(i);
    }  // for
}

void SpectralPlot::updateRainbowSpectralRanges()
{
    for(int num=rainbowNum; num<plot->legend->itemCount(); num++) {
        QColor color = plot->graph(num)->brush().color();
//        color.setAlpha(rainbowTransparency);
        plot->graph(num)->setBrush(QBrush(QColor(color.red(),color.green(),color.blue(),rainbowTransparency)));
    }  // for
}

void SpectralPlot::mousePress(QMouseEvent *event)
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
      plot->axisRect()->setRangeDrag(plot->xAxis->orientation());
    else if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
      plot->axisRect()->setRangeDrag(plot->yAxis->orientation());
    else
      plot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    if (event->modifiers() != Qt::ControlModifier)
        tracer->setStyle(QCPItemTracer::tsNone);
}

void SpectralPlot::mouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
      plot->axisRect()->setRangeZoom(plot->xAxis->orientation());
    else if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
      plot->axisRect()->setRangeZoom(plot->yAxis->orientation());
    else
        plot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void SpectralPlot::mouseMove(QMouseEvent *event)  {
  if (plot->legend->selectTest(event->pos(), false) >= 0) {
    if (plot->cursor().shape() != Qt::ArrowCursor)
      plot->setCursor(Qt::ArrowCursor);
    else if (plot->cursor().shape() != Qt::CrossCursor)
      plot->setCursor(Qt::CrossCursor);
  }  // switch legend

// tracep ToolTip
    MouseOverPlotHeight(event);

}

void SpectralPlot::MouseOverPlotHeight(QMouseEvent *event)
{
    QCustomPlot *curPlot = plot;
    QString strNameX = QString(u8" \u03BB = ");
    QString strNameY = " r = ";
    DisplayCurveData(event, curPlot, strNameX, strNameY);
}

void SpectralPlot::spectralSetAllRange()
{
    sXmin = INT_MAX;  sXmax = INT_MIN;
    sYmin = .0;  sYmax = INT_MIN;
    foreach(zGraph *item, graph_list) {
        if (!item->isVisible()) continue;
        sXmin = std::min(sXmin, *std::min_element(item->keys.begin(), item->keys.end()));
        sXmax = std::max(sXmax, *std::max_element(item->keys.begin(), item->keys.end()));
        sYmax = std::max(sYmax, *std::max_element(item->values.begin(), item->values.end()));
        sYmax = std::max(sYmax, *std::max_element(item->values_std_dev_upper.begin(), item->values_std_dev_upper.end()));
    }  // foreach

    plot->xAxis->setRange(sXmin, sXmax);
    plot->yAxis->setRange(sYmin, sYmax);
    plot->xAxis2->setRange(sXmin, sXmax);
    plot->yAxis2->setRange(sYmin, sYmax);
    plot->replot();

}

void SpectralPlot::selectionChanged()
{
    /*
     normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
     the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
     and the axis base line together. However, the axis label shall be selectable individually.

     The selection state of the left and right axes shall be synchronized as well as the state of the
     bottom and top axes.

     Further, we want to synchronize the selection of the graphs with the selection state of the respective
     legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
     or on its legend item.
    */

    // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || plot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        plot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || plot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
      plot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
      plot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || plot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        plot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || plot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
      plot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
      plot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i<plot->graphCount(); ++i)
    {

      QCPGraph *graph = plot->graph(i);
      QCPPlottableLegendItem *item = plot->legend->itemWithPlottable(graph);

      if (item == nullptr) continue;
      if (item->selected() || graph->selected())
      {
        item->setSelected(true);
        graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
      }
    }
}

void SpectralPlot::DisplayCurveData(QMouseEvent *event, QCustomPlot *curPlot, QString strNameX, QString strNameY)
{
    QCPAbstractPlottable *plottable = curPlot->plottableAt(event->localPos());
    if(plottable)
    {
        if (plottable->name().indexOf("lw") != -1) return;
        if (plottable->name().indexOf("up") != -1) return;

        double x = curPlot->xAxis->pixelToCoord(event->localPos().x());
        double y = curPlot->yAxis->pixelToCoord(event->localPos().y());

        QCPGraph *graph =  qobject_cast<QCPGraph*>(plottable);

// tracer_tolltip
        if (graph) {

            tracer_tolltip->setStyle(QCPItemTracer::tsCircle);
            tracer_tolltip->setGraph(graph);
            tracer_tolltip->setGraphKey(plot->xAxis->pixelToCoord(event->pos().x()));
            tracer_tolltip->updatePosition();
            plot->replot();
            double key = tracer_tolltip->position->key();
            double value = tracer_tolltip->position->value();
            QString key_str = QString(u8"%1 \u043D\u043C").arg(key);
            QFont f = curPlot->font();  f.setPointSize(8);
            QToolTip::setFont(f);
            QToolTip::showText(event->globalPos(),
                               QString(" %1\n%2%3\n%4%5").arg(plottable->name()).arg(strNameX)
                               .arg(key_str).arg(strNameY).arg(value), curPlot, curPlot->rect());
/*            QToolTip::showText(event->globalPos(),
            tr("<table>"
               "<tr>"
                "<td>%L1</td>"
               "</tr>"
               "<tr>"
                "<td>%L2</td>" "<td>%L3</td>"
               "</tr>"
               "<tr>"
                "<td>%L4</td>" "<td>%L5</td>"
               "</tr>"
               "</table>").arg(plottable->name()).arg(strNameX).arg(key_str).arg(strNameY).arg(value), curPlot, curPlot->rect());   */
        }  // if (graph)

/*        if (graph)                    https://www.qcustomplot.com/index.php/support/forum/183
        }  // if (graph)        */
    }
    else {
        QToolTip::hideText();
        tracer_tolltip->setStyle(QCPItemTracer::tsNone);
        tracer_tolltip->setGraph(nullptr);
        plot->replot();
    }
}

QString SpectralPlot::getWritableLocation()
{
    QFileInfo info(f_data_file_name);
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    writableLocation += "/" + info.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    return writableLocation;
}

void SpectralPlot::setStdVevProfiles()
{
    int legeng_min = plot->legend->itemCount();
    foreach(zGraph *item, graph_list) {
        if (!item->isVisible()) continue;
        sYmax = std::max(sYmax, *std::max_element(item->values_std_dev_upper.begin(), item->values_std_dev_upper.end()));

        QCPGraph *graph_lower = plot->addGraph();  graph_lower->setName(item->getTitle() + " lw");
        graph_lower->setPen(Qt::NoPen);
        graph_lower->setSelectable(QCP::stNone);

        QCPGraph *graph_upper = plot->addGraph();  graph_upper->setName(item->getTitle() + " up");
        graph_upper->setPen(Qt::NoPen);

        item->transparency = 50;  QColor bc(200,200,200,item->transparency);
        graph_upper->setBrush(bc);
        graph_upper->setSelectable(QCP::stNone);
        graph_upper->setChannelFillGraph(graph_lower);

        graph_lower->setData(item->keys, item->values_std_dev_lower);
        graph_upper->setData(item->keys, item->values_std_dev_upper);

        item->graph_lower = graph_lower;
        item->graph_upper = graph_upper;

    }  // foreach

    int legeng_max = plot->legend->itemCount();

    for(int i = legeng_max - 1; i >= legeng_min; i--) plot->legend->removeItem(i);

    plot->yAxis->setRange(sYmin, sYmax);
    plot->yAxis2->setRange(sYmin, sYmax);

}

void SpectralPlot::setupRainbowCoordinates()
{
    x_rainbow.append(QPointF(700., 635));
    x_rainbow.append(QPointF(560., 520));
    x_rainbow.append(QPointF(490., 450.));
    x_rainbow.append(QPointF(450., 380.));
    x_rainbow.append(QPointF(590., 560.));
    x_rainbow.append(QPointF(520., 490));
    x_rainbow.append(QPointF(635., 590.));
    rgb_rainbow.append(QColor(255, 0, 0, rainbowTransparency));
    rgb_rainbow.append(QColor(0, 255, 0, rainbowTransparency));
    rgb_rainbow.append(QColor(0, 0, 255, rainbowTransparency));
    rgb_rainbow.append(QColor(255, 0, 255, rainbowTransparency));
    rgb_rainbow.append(QColor(255, 255, 0, rainbowTransparency));
    rgb_rainbow.append(QColor(0, 255, 255, rainbowTransparency));
    rgb_rainbow.append(QColor(255, 128, 0, rainbowTransparency));
}

void SpectralPlot::createRainbow()
{
    QVector<double> y(2, rainbowUp);
    foreach(QPointF p, x_rainbow) {
        int num = x_rainbow.indexOf(p);
        QVector<double> x(2);
        x[0] = p.rx();  x[1] = p.ry();
        QCPGraph *graph = plot->addGraph();  graph->setData(x, y);
        graph->setBrush(QBrush(rgb_rainbow[num]));
        graph->setName("rainbow");
    }  // foreach
}

void SpectralPlot::updateRainbow()
{
    for(int i=0; i<plot->graphCount(); i++) {
        QCPGraph *graph = plot->graph(i);
        if (graph->name() != "rainbow") continue;
        QColor color = graph->brush().color();
        if (rainbowCheckBox->isChecked()) color.setAlpha(rainbowTransparency);
        else color.setAlpha(0);
        graph->setBrush(QBrush(color));
    }  // for
    plot->replot();
}


/*
   QToolTip::showText(event->globalPos(), tr("<table>"
   "<tr>"  "<td>%L1:</td>" "<td>%L2</td>"  "</tr>"
   "<tr>"  "<td>%L3:</td>" "<td>%L4</td>"  "</tr>"
   "</table>").arg(strNameX).arg(key).arg(strNameY).arg(value),
   curPlot, curPlot->rect());
*/

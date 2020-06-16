#include "imagespectral.h"

imageSpectral::imageSpectral(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi();
    move(20,20);  resize(600,600);
    setupConnections();

// цвета и стили профилей
    J09::spectralColorType sct;
    sct.color = QColor(255,0,0);  sct.style = 1; spectralColor.append(sct); // red style 1
    sct.color = QColor(0,255,0);  sct.style = 2; spectralColor.append(sct); // green style 2
    sct.color = QColor(0,0,255);  sct.style = 3; spectralColor.append(sct); // blue style 3
    sct.color = QColor(57,57,57);  sct.style = 4; spectralColor.append(sct); // grey style 4

    sct.color = QColor(255,0,0);  sct.style = 5; spectralColor.append(sct); // red style 5
    sct.color = QColor(0,255,0);  sct.style = 6; spectralColor.append(sct); // green style 6
    sct.color = QColor(0,0,255);  sct.style = 7; spectralColor.append(sct); // blue style 7
    sct.color = QColor(57,57,57);  sct.style = 8; spectralColor.append(sct); // grey style 8

    sct.color = QColor(255,0,0);  sct.style = 4; spectralColor.append(sct); // red style 4
    sct.color = QColor(0,255,0);  sct.style = 3; spectralColor.append(sct); // green style 3
    sct.color = QColor(0,0,255);  sct.style = 2; spectralColor.append(sct); // blue style 2
    sct.color = QColor(57,57,57);  sct.style = 1; spectralColor.append(sct); // grey style 1

    sct.color = QColor(255,0,0);  sct.style = 8; spectralColor.append(sct); // red style 8
    sct.color = QColor(0,255,0);  sct.style = 7; spectralColor.append(sct); // green style 7
    sct.color = QColor(0,0,255);  sct.style = 6; spectralColor.append(sct); // blue style 6
    sct.color = QColor(57,57,57);  sct.style = 5; spectralColor.append(sct); // grey style 5

    sct.color = QColor(255,0,0);  sct.style = 2; spectralColor.append(sct); // red style 8
    sct.color = QColor(0,255,0);  sct.style = 4; spectralColor.append(sct); // green style 7
    sct.color = QColor(0,0,255);  sct.style = 1; spectralColor.append(sct); // blue style 6
    sct.color = QColor(57,57,57);  sct.style = 3; spectralColor.append(sct); // grey style 5

}

void imageSpectral::updateData(QList<zGraph *> list, bool rescale)
{
    // data
    plot->clearGraphs();

    sXmin = INT_MAX;  sXmax = INT_MIN;
    sYmin = .0;  sYmax = INT_MIN;
    foreach(zGraph *item, list) {
        if (!item->isVisible()) continue;
        int n = item->profile.count();
        QVector<double> x(n), y(n);
        for (int i=0; i<n; i++) { x[i] = item->profile[i].rx(); y[i] = item->profile[i].ry(); }
        sXmin = std::min(sXmin, *std::min_element(x.begin(), x.end()));
        sXmax = std::max(sXmax, *std::max_element(x.begin(), x.end()));
        sYmax = std::max(sYmax, *std::max_element(y.begin(), y.end()));
        plot->addGraph();
        plot->graph()->setName(item->getTitle());
        plot->graph()->setData(x, y);
         QPen graphPen;

        int num = list.indexOf(item);
        if (num > spectralColor.count() - 1) num -= spectralColor.count();
        graphPen.setColor(spectralColor[num].color);
        graphPen.setWidthF(1.);
        plot->graph()->setPen(graphPen);

        int style = spectralColor[num].style;
        plot->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(style)));

    }  // foreach

//    if (rescale) spectralAnalisysPlot->rescaleAxes();
    if (rescale) spectralSetAllRange();

    // spectral colors
    int count = getGraphCount();
    setRainbowSpectralRanges();
    mRainbow = getGraphCount() - count;

    plot->replot();

}

void imageSpectral::updateDataOneProfile(zGraph *item, int num)
{
    if (num > plot->graphCount() - 1) return;
    tracer->setGraph(0x0);
    tracer->updatePosition();
    int n = item->profile.count();
    QVector<double> x(n), y(n);
    for (int i=0; i<n; i++) { x[i] = item->profile[i].rx(); y[i] = item->profile[i].ry(); }
    plot->graph(num)->setData(x, y);
    plot->replot();
}

void imageSpectral::setRainbowSpectralRanges()
{
    // attempt to nice picture

    int legeng_min = plot->legend->itemCount();

    QVector<double> x(2), y(2);
    double up = 10.;  y[0] = up;   y[1] = up;

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

    // chlorophyll
    x[0] = 675.;  x[1] = 676.;    plot->addGraph();
    plot->graph()->setData(x, y);
    plot->graph()->setBrush(QBrush(QColor(0, 0, 255, 50)));

    int legeng_max = plot->legend->itemCount();

    for(int i = legeng_max - 1; i >= legeng_min; i--) {
//        QCPAbstractLegendItem *item = spectralAnalisysPlot->legend->item(i);
//        item->setVisible(false);
        plot->legend->removeItem(i);
    }  // for

}

bool imageSpectral::eventFilter(QObject *object, QEvent *event)
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
//                QString message = QString("r%1 коэффициент отражения %2\n").arg(value).arg(dataValue);
//                textEdit->insertPlainText(message);
//                statusBar->showMessage(QString("graph '%1' at point #%2 ( %3 nm ) with value %4.").
//                                 arg(graph->name()).arg(tracerIndex).arg(value).arg(dataValue));
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
//                QString message = QString("r%1 коэффициент отражения %2\n").arg(value).arg(dataValue);
//                textEdit->insertPlainText(message);
//                statusBar->showMessage(QString("graph '%1' at point #%2 ( %3 nm ) with value %4.").
//                                 arg(graph->name()).arg(tracerIndex).arg(value).arg(dataValue));
                auto message = getGraphClickedStrings(tracerIndex, graph->name(), value, dataValue);
                textEdit->insertPlainText(message.at(0));
                statusBar->showMessage(message.at(1));

            }  // if
            return true;
        }  // if Key_Right
    }  // if QEvent::KeyPress

    return QDockWidget::eventFilter(object, event);
}

void imageSpectral::setupUi()
{
    setObjectName("imageSpectral");
    setWindowTitle("Спектральный анализ");
    setAllowedAreas(0);  setFloating(true);

    installEventFilter(this);

    resize(900,600);  move(500,50);
    bottomGroupBox = new QGroupBox("");
    gridLayout = new QGridLayout;
    textEdit = new QTextEdit;
    textEdit->resize(500,100);
    textEdit->setMinimumWidth(400);
    gridLayout->addWidget(textEdit, 0, 0, Qt::AlignLeft);
    axisResizeButton = new QPushButton("Полный масштаб");
    gridLayout->addWidget(axisResizeButton, 0, 1, Qt::AlignLeft);

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

    // Инициализируем трассировщик
    tracer = new QCPItemTracer(plot);
    // Interactions
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    plot->axisRect()->setupFullAxesBox();
    plot->plotLayout()->insertRow(0);

    QCPTextElement *title = new QCPTextElement(plot,
                    "Спектральные профили областей интереса");
    plot->plotLayout()->addElement(0, 0, title);
    plot->xAxis->setLabel("длина волны, нм");
//    spectralAnalisysPlot->xAxis->setSelectableParts(QCPAxis::spAxis);
    plot->yAxis->setLabel("коэффициент отражения");
    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignTop);
    QFont legendFont = font();
    legendFont.setPointSize(7);
    plot->legend->setFont(legendFont);
    plot->legend->setSelectedFont(legendFont);
    plot->legend->setSelectableParts(QCPLegend::spItems);
    plot->legend->setRowSpacing(0);

}

void imageSpectral::setupConnections()
{
    // allRange - need corrected
    connect(axisResizeButton, &QPushButton::clicked, this, &imageSpectral::spectralSetAllRange);

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

}

void imageSpectral::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
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

QStringList imageSpectral::getGraphClickedStrings(int dataIndex, QString name, double keyValue, double dataValue) {

    QStringList strlist;
    strlist.append(QString("x = r%1 нм y = %2 * ").arg(keyValue).arg(dataValue));
    strlist.append(QString("Спектр '%1' точка #%2 ( дл.волны %3 нм ) коэффициент отражения %4").
                   arg(name).arg(dataIndex).arg(keyValue).arg(dataValue));
    return strlist;
}

void imageSpectral::mousePress(QMouseEvent *event)
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

void imageSpectral::mouseWheel()
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

void imageSpectral::mouseMove(QMouseEvent *event)
{
    switch (plot->legend->selectTest(event->pos(), false) >= 0) {
    case true : if (plot->cursor().shape() != Qt::ArrowCursor)
            plot->setCursor(Qt::ArrowCursor);
        break;
    case false : if (plot->cursor().shape() != Qt::CrossCursor)
            plot->setCursor(Qt::CrossCursor);
        break;
    }  // switch legend

}

void imageSpectral::spectralSetAllRange()
{
    plot->xAxis->setRange(sXmin, sXmax);
    plot->yAxis->setRange(sYmin, sYmax);
    plot->xAxis2->setRange(sXmin, sXmax);
    plot->yAxis2->setRange(sYmin, sYmax);
    plot->replot();
}

void imageSpectral::selectionChanged()
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

      if (item == 0x0) continue;
      if (item->selected() || graph->selected())
      {
        item->setSelected(true);
        graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
      }
    }
}

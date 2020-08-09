#include "imagemask.h"

imageMask::imageMask(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi();
    move(750,500); resize(700,400);
//    hide();

}

void imageMask::setPreviewPixmap(QPixmap &mainRGB)
{
    previewRGB = &mainRGB;
}

void imageMask::setLWidgetAndIHandler(QListWidget *lw, ImageHandler *ih)
{
    maskListWidget = lw;
    imgHand = ih;

    maskListWidget->setIconSize( defaultIconSize );
    maskListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    maskListWidget->setViewMode( QListWidget::IconMode );

}

QListWidgetItem *imageMask::createMaskWidgetItem(const QString &atext, const QString &atoolTip, const QIcon &aicon)
{
    QListWidgetItem *item = new QListWidgetItem(maskListWidget);
    item->setText(atext); item->setToolTip(atoolTip);
    item->setIcon(aicon);
    item->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    return item;
}

void imageMask::setupUi()
{
    setObjectName("mask");
    setWindowTitle("Калькулятор масочных изображений");
    setAllowedAreas(nullptr);
    setFloating(true);

// размещение на форме интерфейсных элементов
// порядок размещения - сверху вниз

    QWidget *centralWidget = new QWidget(this);
    setWidget(centralWidget);
    QHBoxLayout  *mainHorzLayout = new QHBoxLayout(centralWidget);  // главный горизонтальный шаблон
    QGroupBox *maskGroupBox = new QGroupBox(tr("Масочное изображение (просмотр или результат расчета)"));
    mainHorzLayout->addWidget(maskGroupBox, 45);
// вертикальный шаблон для 4 масок и калькулятора и двух статус строк
    QVBoxLayout  *mask4andCalculatorLayout = new QVBoxLayout(centralWidget);  // правый вертикальный шаблон
    mainHorzLayout->addLayout(mask4andCalculatorLayout, 55);
// левая часть верхнего шаблона
    plot = new QCustomPlot(maskGroupBox);
    QGridLayout *plotLayout = new QGridLayout(maskGroupBox);  // добавление плоттера в шаблон
    plotLayout->addWidget(plot);
    plot->setToolTip("Окно отображения результата\n"
                     "арифметических операций с масками.\n"
                     "Для масштабирования используйте\n"
                     "перетаскивание мышью\n"
                     "и колесо мыши");
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot->addLayer("image");
    image_pixmap = new QCPItemPixmap(plot);
    image_pixmap->setVisible(true);
    image_pixmap->setLayer("image");
    image_pixmap->setScaled(true);

    defaultRGB.fill(QColor(125,125,125,50));
    setDefaultRGB();
// КАЛЬКУЛЯТОР
    QGroupBox *calculatorGroupBox = new QGroupBox(tr("Калькулятор"));

    mask4andCalculatorLayout->addWidget(calculatorGroupBox,5);

    QGridLayout *calculatorLayout = new QGridLayout(calculatorGroupBox);
    Button *additionButton = createButton(tr("+"), tr("1 + 1 = 1\n1 + 0 = 1\n0 + 1 = 1\n0 + 0 = 0"), SLOT(plug()));
    calculatorLayout->addWidget(additionButton, 0, 0);
    Button *subtractionButton = createButton(tr("-"), tr("1 - 1 = 0\n1 - 0 = 1\n0 - 1 = 0\n0 - 0 = 0"), SLOT(plug()));
    calculatorLayout->addWidget(subtractionButton, 0, 1);
    Button *cancelButton = createButton(tr("Сброс"), tr("Сброс всех операций\nДля нового расчета выполните:\n"
                                                        "1. выберите маску\n"
                                                        "2. нажмите кнопку операции\n"
                                                        "3. выберите вторую маску"), SLOT(cancel()));
    calculatorLayout->addWidget(cancelButton, 1, 1);
    Button *saveButton = createButton(tr("Сохранить ..."), tr("Сохранить в списке масок ..."), SLOT(plug()));
    calculatorLayout->addWidget(saveButton, 0, 2);
    Button *closeButton = createButton(tr("Закрыть"), tr("Закрыть окно"), SLOT(hide()));
    calculatorLayout->addWidget(closeButton, 1, 3);
    Button *clearButton = createButton(tr("Очистить"), tr("Очистить окно маска-результат\nи иконки изображений-масок"), SLOT(plug()));
    calculatorLayout->addWidget(clearButton, 0, 3);

    formulaLabel->setText("here will by a formula here will by a formula here will by a formula here will by a formula");
    mask4andCalculatorLayout->addWidget(formulaLabel,1);
    showModeLabel->setText(getResultShowModesString());
    mask4andCalculatorLayout->addWidget(showModeLabel,1);

}

void imageMask::setDefaultRGB()
{
    image_pixmap->topLeft->setCoords(0,defaultRGB.height());
    image_pixmap->bottomRight->setCoords(defaultRGB.width(),0);

    plot->xAxis->setRange(0,defaultRGB.width());
    plot->yAxis->setRange(0,defaultRGB.height());

    image_pixmap->setPixmap(defaultRGB);
}

QString imageMask::getResultShowModesString()
{
    return resultShowModesStr.arg(resultShowModesList.at(show_mode));
}

Button *imageMask::createButton(const QString &text, const QString &tooltip, const char *member)
{
    Button *button = new Button(text);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, member);
    return button;
}

QLabel *imageMask::createHeaderLabel(const QString &text)
{
    QLabel *label = new QLabel(text);
    label->setAlignment(Qt::AlignCenter);
    return label;
}

DropArea *imageMask::createPixmapLabel()
{
    DropArea *label = new DropArea;
    return label;
}

void imageMask::plug(){}

Button::Button(const QString &text, QWidget *parent)
    : QToolButton(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setText(text);
}

DropArea::DropArea(QWidget *parent)
    : QLabel(parent)
{
    setMinimumSize(132, 132);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
    setAcceptDrops(true);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    clear();
}

void DropArea::clear()
{
    setText(tr("<Перетащите сюда\nизображение маску>"));
    setBackgroundRole(QPalette::Dark);
}

void DropArea::dragEnterEvent( QDragEnterEvent* event ) {

    clear();
    setBackgroundRole(QPalette::Highlight);
    event->acceptProposedAction();
}

void DropArea::dragLeaveEvent(QDragLeaveEvent *event) {
    if (empty) setBackgroundRole(QPalette::Dark);
    else {
    int h=this->height();
    int w=this->width();
    this->setPixmap(image->scaled(w,h,Qt::IgnoreAspectRatio, Qt::SmoothTransformation));}

    event->accept();
}

void DropArea::dropEvent(QDropEvent *event)
{
    empty = false;

    int cr = listWidget->currentRow();
    title->setText(QString("num = %1").arg(cr));

    int h=this->height();
    int w=this->width();
    this->setPixmap(image->scaled(w,h,Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    setBackgroundRole(QPalette::Dark);
    event->acceptProposedAction();
}

void DropArea::mousePressEvent(QMouseEvent *ev)
{
    image_pixmap->setPixmap(*image);
    image_pixmap->topLeft->setCoords(0,image->height());
    image_pixmap->bottomRight->setCoords(image->width(),0);
    plot->rescaleAxes();
    plot->replot();
}



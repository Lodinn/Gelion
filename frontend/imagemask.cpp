#include "imagemask.h"

imageMask::imageMask(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi();
    move(750,200); resize(900,480);
    installEventFilter(this);
//    hide();

}

void imageMask::setPreviewPixmap(QPixmap &mainRGB)
{
    previewRGB = &mainRGB;
}

void imageMask::setLWidgetAndIHandler(QListWidget *lw, ImageHandler *ih)
{
    rotation = gsettings->main_rgb_rotate_start;
    maskListWidget = lw;
    imgHand = ih;

    maskListWidget->setIconSize( defaultIconSize );
    maskListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    maskListWidget->setViewMode( QListWidget::IconMode );

    foreach(DropArea *da, pixmapLabelsVector) {
        da->listWidget = maskListWidget;
        da->imgHand = imgHand;
        da->rotation = rotation;
    }  // foreach

}

QListWidgetItem *imageMask::createMaskWidgetItem(J09::maskRecordType *am, QImage &img)
{
    QListWidgetItem *item = new QListWidgetItem(maskListWidget);
    item->setText(am->title); item->setToolTip(am->formula);

    QMatrix rm;    rm.rotate(am->rotation);
    item->setIcon(QIcon(QPixmap::fromImage(img).transformed(rm)));

    item->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    return item;
}

void imageMask::set4MasksToForm()
{
    int count = 0;
    for(int row=0; row<maskListWidget->count();row++) {
        QListWidgetItem *item = maskListWidget->item(row);
        if (item->checkState() != Qt::Checked) continue;
        if (count>pixmapLabelsVector.count()-1) return;
        pixmapLabelsVector[count]->setNum(row);  count++;

    }  // for
}

void imageMask::setupUi()
{

    setObjectName("mask");
    setWindowTitle("Калькулятор масочных изображений");
    setAllowedAreas(nullptr);
    setFloating(true);
    installEventFilter(this);

// размещение на форме интерфейсных элементов
// порядок размещения - сверху вниз

    QWidget *centralWidget = new QWidget(this);
    setWidget(centralWidget);
    QHBoxLayout  *mainHorzLayout = new QHBoxLayout(centralWidget);  // главный горизонтальный шаблон
    QGroupBox *maskGroupBox = new QGroupBox(tr("Масочное изображение (просмотр или результат расчета)"));
    mainHorzLayout->addWidget(maskGroupBox, 50);
// вертикальный шаблон для 4 масок и калькулятора и двух статус строк
    QVBoxLayout  *mask4andCalculatorLayout = new QVBoxLayout(centralWidget);  // правый вертикальный шаблон
    mainHorzLayout->addLayout(mask4andCalculatorLayout, 50);
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

// МАСКИ - ИКОНКИ
    QGridLayout *maskIconsLayout = new QGridLayout(centralWidget);

    mask4andCalculatorLayout->addLayout(maskIconsLayout,20);

    for (int row = 0; row < mask_row_count; ++row) {
        for (int column = 0; column < mask_col_count; ++column) {
            pixmapLabels[column][row] = createPixmapLabel();
            DropArea *da = pixmapLabels[column][row];
            da->title = createHeaderLabel(defTitleString);
            maskIconsLayout->addWidget(da->title, 2 * row, column);
            maskIconsLayout->addWidget(da, 2 * row + 1, column);
            da->plot = plot;
            da->image_pixmap = image_pixmap;
            da->result = &result;
            pixmapLabelsVector.append(da);
            connect(da, &DropArea::exec, this, &imageMask::maskModify);
        }
    }
    maskIconsLayout->setRowStretch(0,1);        maskIconsLayout->setRowStretch(1,10);
    maskIconsLayout->setRowStretch(2,1);        maskIconsLayout->setRowStretch(3,10);
    maskIconsLayout->setColumnStretch(0,50);    maskIconsLayout->setColumnStretch(1,50);

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
                                                        "3. выберите вторую маску"), SLOT(plug()));
    calculatorLayout->addWidget(cancelButton, 1, 1);
    Button *saveButton = createButton(tr("Сохранить ..."), tr("Сохранить в списке масок ..."), SLOT(plug()));
    calculatorLayout->addWidget(saveButton, 0, 2);
    Button *closeButton = createButton(tr("Закрыть"), tr("Закрыть окно"), SLOT(hide()));  // HIDE
    calculatorLayout->addWidget(closeButton, 1, 3);
    Button *clearButton = createButton(tr("Очистить"), tr("Очистить окно маска-результат\nи иконки изображений-масок"), SLOT(plug()));
    calculatorLayout->addWidget(clearButton, 0, 3);

    formulaLabel->setText("here will by a formula here will by a formula here will by a formula here will by a formula");
    mask4andCalculatorLayout->addWidget(formulaLabel,1);
    showModeLabel->setText(getResultShowModesString());
    mask4andCalculatorLayout->addWidget(showModeLabel,1);

}

bool imageMask::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);

        if( key->key() == Qt::Key_A ) { routate90(false); return true; }  // A
        if( key->key() == Qt::Key_S ) { routate90(true); return true; }  // S

    }  // KeyPress
    return QDockWidget::eventFilter(object, event);
}

void imageMask::setDefaultRGB()
{
    image_pixmap->topLeft->setCoords(0,defaultRGB.height());
    image_pixmap->bottomRight->setCoords(defaultRGB.width(),0);

    plot->xAxis->setRange(0,defaultRGB.width());
    plot->yAxis->setRange(0,defaultRGB.height());

    image_pixmap->setPixmap(defaultRGB);
    plot->replot();
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

void imageMask::routate90(bool clockwise)
{
    if (clockwise) {
        rotation += 90.;
        if (rotation > 360.) rotation -= 360.;
    } else {
        rotation -= 90.;
        if (rotation < 0.) rotation += 360.;
    }  // if

    updateMainPreviewWithIconsPreviw();
}

void imageMask::updateMainPreviewWithIconsPreviw()
{
    foreach(DropArea *da, pixmapLabelsVector) {
        da->rotation = rotation;
        da->setNum(da->getNum());
    }  // foreach

    QMatrix rm;    rm.rotate(rotation);
    QImage img = get_mask_image();
    image_pixmap->setPixmap(QPixmap::fromImage(img).transformed(rm));
    plot->replot();
}

void imageMask::plug(){}

void imageMask::maskModify(int num)
{
    if (im == imageMask::imNone) return;
    switch (im) {
    case imageMask::imAddition : doAddition(num);
        break;
    case imageMask::imSubtraction : doSubtraction(num);
        break;
    }
}


QImage imageMask::get_mask_image()
{
    QSize slice_size(result[0].count(), result.count());
    QImage mask_img(slice_size, QImage::Format_Mono);

        for(int y = 0; y < slice_size.height(); y++)
            for(int x = 0; x < slice_size.width(); x++) {
                mask_img.setPixel(x, y, result[y][x]);
            }  // for

        return mask_img;
}

void imageMask::doAddition(int num)
{
    QSize slice_size(result[0].count(), result.count());
    J09::maskRecordType *mask = imgHand->current_image()->getMask(num);

    QVector<int8_t> line(slice_size.width(), 1);
    QVector<QVector<int8_t> > buff(slice_size.height(), line);

    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++)
            if(result[y][x] == 0 && mask->mask[y][x] == 0) buff[y][x] = 0;
    result = buff;
}

void imageMask::doSubtraction(int num)
{
    QSize slice_size(result[0].count(), result.count());
    J09::maskRecordType *mask = imgHand->current_image()->getMask(num);

    QVector<int8_t> line(slice_size.width(), 0);
    QVector<QVector<int8_t> > buff(slice_size.height(), line);

    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++)
            if(result[y][x] == 1 && mask->mask[y][x] == 0) buff[y][x] = 1;
    result = buff;
}

QString imageMask::getMaskDataFileName(QString data_file_name)
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(data_file_name);
    writableLocation += "/" + info.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    return writableLocation + "/" + defMaskFileDataName;
}

void imageMask::loadMasksFromFile(QString data_file_name)
{
    if (data_file_name.isEmpty()) return;

    QFile datfile(getMaskDataFileName(data_file_name));
    datfile.open(QIODevice::ReadOnly);
    QDataStream datstream( &datfile );
    datstream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    datstream.setByteOrder(QDataStream::LittleEndian);
    int count;
    datstream >> count;
    for(int i=0; i<count; i++) {
        J09::maskRecordType *am = new J09::maskRecordType;
        imgHand->current_image()->append_mask(am);
        datstream >> am->checked >> am->title >> am->formula >> am->invers;
        datstream >> am->formula_step_by_step >> am->img >> am->brightness >> am->rotation;
        datstream >> am->mask;
    }
    datfile.close();
}

void imageMask::updateMaskListWidget()
{
    maskListWidget->clear();
    int count = imgHand->current_image()->getMasksCount();
    for(int i=0; i<count; i++) {
        J09::maskRecordType *am = imgHand->current_image()->getMask(i);
        QListWidgetItem *item = this->createMaskWidgetItem(am, am->img);
        maskListWidget->addItem(item);
    }
}

void imageMask::saveMasksToFile(QString data_file_name)
{
    if (data_file_name.isEmpty()) return;

    QFile datfile(getMaskDataFileName(data_file_name));
    datfile.open(QIODevice::WriteOnly);
    QDataStream datstream( &datfile );

    datstream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    datstream.setByteOrder(QDataStream::LittleEndian);
    int count = imgHand->current_image()->getMasksCount();
    datstream << count;
    for(int i=0; i<count; i++) {
        J09::maskRecordType *m = imgHand->current_image()->getMask(i);
        datstream << m->checked << m->title << m->formula << m->invers;
        datstream << m->formula_step_by_step << m->img << m->brightness << m->rotation;
        datstream << m->mask;
    }
    datfile.close();
}


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
    clear(false);
}

void DropArea::setNum(int n)
{
    num = n;

    if (n<0) {
        empty = true;
        clear(true);
        return;
    }
    empty = false;

    J09::maskRecordType *mask = imgHand->current_image()->getMask(num);

    QMatrix rm;    rm.rotate(rotation);
    QImage img = imgHand->current_image()->get_mask_image(num);
    pixmap = QPixmap::fromImage(img).transformed(rm);

    int h=this->height();
    int w=this->width();
    this->setPixmap(pixmap.scaled(w,h,Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    title->setText(mask->title);
    setToolTip(mask->formula);
}

void DropArea::clear(bool full)
{
    setText(tr("<Перетащите сюда\nмасочное изображение>"));
    setToolTip(tr("Место для уменьшенного\nмасочного изображения"));
    setBackgroundRole(QPalette::Dark);
    if (full) title->setText(defTitleString);
}

void DropArea::dragEnterEvent( QDragEnterEvent* event ) {

    clear(true);
    setBackgroundRole(QPalette::Highlight);
    event->acceptProposedAction();
}

void DropArea::dragLeaveEvent(QDragLeaveEvent *event) {
    if (empty)
        setBackgroundRole(QPalette::Dark);
    else
        setNum(num);

    event->accept();
}

void DropArea::dropEvent(QDropEvent *event)
{
    empty = false;

    int cr = listWidget->currentRow();

    setNum(cr);

    setBackgroundRole(QPalette::Dark);
    event->acceptProposedAction();
}

void DropArea::mousePressEvent(QMouseEvent *ev)
{
    if (empty) return;

    image_pixmap->setPixmap(pixmap);
    image_pixmap->topLeft->setCoords(0,pixmap.height());
    image_pixmap->bottomRight->setCoords(pixmap.width(),0);
    plot->rescaleAxes();
    plot->replot();
    result->clear();
    QVector<QVector<int8_t> > v(3,QVector<int8_t>(5));
    result->append(v);

    emit exec(num);

}



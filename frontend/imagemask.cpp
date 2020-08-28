#include "imagemask.h"

imageMask::imageMask(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi();
    move(750,200); resize(800,600);
    installEventFilter(this);
}

void imageMask::setPreviewPixmap(QPixmap &mainRGB)
{
    previewRGB = &mainRGB;
}

void imageMask::clearForAllObjects()
{
    maskListWidget->clear();
    imgHand->current_image()->deleteAllMasks();
    clear();  hide();
}

void imageMask::setLWidgetAndIHandler(QListWidget *lw, ImageHandler *ih)
{
    rotation = gsettings->main_rgb_rotate_start;
    result.rotation = rotation;
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

int imageMask::set2MasksToForm()
{
    int count = 0;
    for(int row=0; row<maskListWidget->count();row++) {
        QListWidgetItem *item = maskListWidget->item(row);
        if (item->checkState() != Qt::Checked) continue;
        if (count>pixmapLabelsVector.count()-1) return count;
        pixmapLabelsVector[count]->setNumA(row);  count++;
    }  // for
    return count;
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
    QHBoxLayout *mainHorzLayout = new QHBoxLayout(centralWidget);  // главный горизонтальный шаблон
    maskGroupBox = new QGroupBox(tr("Изображение"));
    mainHorzLayout->addWidget(maskGroupBox, 3);  //*************main
    // вертикальный шаблон для 2 масок и калькулятора и двух статус строк
    QVBoxLayout *mask2andCalculatorLayout = new QVBoxLayout(centralWidget);  // правый вертикальный шаблон
    mainHorzLayout->addLayout(mask2andCalculatorLayout, 2);  //*************main

    plot = new QCustomPlot(maskGroupBox);
    QVBoxLayout *plotLayout = new QVBoxLayout(maskGroupBox);  // добавление плоттера
    maskGroupBox->setLayout(plotLayout);

    plotLayout->addWidget(plot, 10);
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
    setPixmapToPlot(defaultRGB);

    QGridLayout *titleGrid = new QGridLayout(maskGroupBox);
    plotLayout->addLayout(titleGrid, 3);
    titleLabel = new QLabel("Наименование", maskGroupBox);
    formulaLabel = new QLabel("Формула", maskGroupBox);

    titleEdit = new QLineEdit("Наименование", maskGroupBox);
    formulaEdit = new QTextEdit("Формула", maskGroupBox);
    formulaEdit->setReadOnly(true);

    infoLabel = new QLabel(getResultShowModesString(), maskGroupBox);
    saveButton = createButton(tr("Сохранить"), tr("Сохранить в списке масок"), SLOT(plug()));
    clearButton = createButton(tr("Очистить"), tr("Очистить изображение\nи иконки изображений-масок"), SLOT(clear()));
    closeButton = createButton(tr("Закрыть"), tr("Закрыть окно"), SLOT(hide()));  // HIDE

    titleGrid->addWidget(saveButton, 0, 2);  titleGrid->addWidget(clearButton, 1, 2, Qt::AlignTop);
    titleGrid->addWidget(closeButton, 2, 2);

    titleGrid->addWidget(titleLabel, 0, 0);
    titleGrid->addWidget(titleEdit, 0, 1);
    titleGrid->addWidget(formulaLabel, 1, 0, Qt::AlignTop);
    titleGrid->addWidget(formulaEdit, 1, 1);
    titleGrid->addWidget(infoLabel, 2, 0, 1, 2, Qt::AlignLeft);

    titleGrid->setRowStretch(0,3);  titleGrid->setRowStretch(1,5);
    titleGrid->setColumnStretch(0,10);
    titleGrid->setColumnStretch(1,50);
    titleGrid->setColumnStretch(2,10);

    calculatorGroupBox = new QGroupBox(tr("Калькулятор [сверху-вниз ; +Ctrl снизу-вверх]"));
    QHBoxLayout *calculatorLayout = new QHBoxLayout(calculatorGroupBox);
    calculatorGroupBox->setLayout(calculatorLayout);

    for(int i=0; i<2; i++) {
        DropArea *da = createPixmapLabel();
        da->title = createHeaderLabel(defTitleString);  // up\down preview
        mask2andCalculatorLayout->addWidget(da->title, 1);
        mask2andCalculatorLayout->addWidget(da, 15);
        da->plot = plot;  da->image_pixmap = image_pixmap;
        pixmapLabelsVector.append(da);

        if (i == 0) mask2andCalculatorLayout->addWidget(calculatorGroupBox, 2);
    }

    addition = new QRadioButton(tr("Добавление"));
    addition->setToolTip(tr("1 + 1 = 1\n1 + 0 = 1\n0 + 1 = 1\n0 + 0 = 0"));
    subtraction = new QRadioButton(tr("Вычитание"));
    subtraction->setToolTip(tr("1 - 1 = 0\n1 - 0 = 1\n0 - 1 = 0\n0 - 0 = 0"));
    clipping = new QRadioButton(tr("Отсечение"));
    clipping->setToolTip(tr("1 cl 1 = 1\n1 cl 0 = 0\n0 cl 1 = 0\n0 cl 0 = 0"));

    calculationButtons.append(addition);addition->installEventFilter(this);
    calculationButtons.append(subtraction);subtraction->installEventFilter(this);
    calculationButtons.append(clipping);clipping->installEventFilter(this);

    calculatorLayout->addWidget(addition, 1);
    calculatorLayout->addWidget(subtraction, 1);
    calculatorLayout->addWidget(clipping, 1);
    calculatorLayout->addStretch(1);

    addition->setChecked(true);

    setButtonsEnabled(false);

    // CONNECTIONS
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot, &QCustomPlot::customContextMenuRequested, this, &imageMask::contextMenuRequest);
    foreach(DropArea *da, pixmapLabelsVector)
        connect(da, &DropArea::exec, this, &imageMask::execSlot);
    connect(this, &imageMask::m_exec, this, &imageMask::execSlot);


    //    connect(clearButton, &Button::pressed, this, &imageMask::clear);

    return;


// размещение на форме интерфейсных элементов
// порядок размещения - сверху вниз

//    QWidget *centralWidget = new QWidget(this);
//    setWidget(centralWidget);
//    QHBoxLayout  *mainHorzLayout = new QHBoxLayout(centralWidget);  // главный горизонтальный шаблон
//    QGroupBox *maskGroupBox = new QGroupBox(tr("Масочное изображение (просмотр или результат расчета)"));
    mainHorzLayout->addWidget(maskGroupBox, 50);
// вертикальный шаблон для 4 масок и калькулятора и двух статус строк
    QVBoxLayout  *mask4andCalculatorLayout = new QVBoxLayout(centralWidget);  // правый вертикальный шаблон
    mainHorzLayout->addLayout(mask4andCalculatorLayout, 50);
// левая часть верхнего шаблона
    plot = new QCustomPlot(maskGroupBox);
//    QGridLayout *plotLayout = new QGridLayout(maskGroupBox);  // добавление плоттера
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
            pixmapLabelsVector.append(da);
//            connect(da, &DropArea::exec, this, &imageMask::maskModify);
        }
    }
    maskIconsLayout->setRowStretch(0,1);        maskIconsLayout->setRowStretch(1,10);
    maskIconsLayout->setRowStretch(2,1);        maskIconsLayout->setRowStretch(3,10);
    maskIconsLayout->setColumnStretch(0,50);    maskIconsLayout->setColumnStretch(1,50);

// КАЛЬКУЛЯТОР
//    QGroupBox *calculatorGroupBox = new QGroupBox(tr("Калькулятор"));

    mask4andCalculatorLayout->addWidget(calculatorGroupBox,5);

//    QGridLayout *calculatorLayout = new QGridLayout(calculatorGroupBox);
//    additionButton = createButton(tr("+"), tr("1 + 1 = 1\n1 + 0 = 1\n0 + 1 = 1\n0 + 0 = 0"), SLOT(plug()));
//    calculatorLayout->addWidget(additionButton, 0, 0);
//    subtractionButton = createButton(tr("-"), tr("1 - 1 = 0\n1 - 0 = 1\n0 - 1 = 0\n0 - 0 = 0"), SLOT(plug()));
//    calculatorLayout->addWidget(subtractionButton, 0, 1);
//    cancelButton = createButton(tr("Сброс"), tr("Сброс всех операций\nДля нового расчета выполните:\n"
//                                                        "1. выберите маску\n"
//                                                        "2. нажмите кнопку операции\n"
//                                                        "3. выберите вторую маску"), SLOT(plug()));
//    calculatorLayout->addWidget(cancelButton, 1, 1);
//    saveButton = createButton(tr("Сохранить ..."), tr("Сохранить в списке масок ..."), SLOT(plug()));
//    calculatorLayout->addWidget(saveButton, 0, 2);
//    closeButton = createButton(tr("Закрыть"), tr("Закрыть окно"), SLOT(hide()));  // HIDE
//    calculatorLayout->addWidget(closeButton, 1, 3);
//    clearButton = createButton(tr("Очистить"), tr("Очистить окно маска-результат\nи иконки изображений-масок"), SLOT(plug()));
//    calculatorLayout->addWidget(clearButton, 0, 3);

    formulaLabel->setText("Формуля операции по форме $наименование 1$ + $наименование 2$");
    mask4andCalculatorLayout->addWidget(formulaLabel,1);
    showModeLabel->setText(getResultShowModesString());
    mask4andCalculatorLayout->addWidget(showModeLabel,1);

// CONNECTIONS
//    connect(clearButton, &Button::pressed, this, &imageMask::clear);
//    connect(additionButton, &Button::pressed, this, &imageMask::addition);
//    connect(subtractionButton, &Button::pressed, this, &imageMask::subtraction);
//    connect(cancelButton, &Button::pressed, this, &imageMask::cancel);
}

bool imageMask::eventFilter(QObject *object, QEvent *event)
{
   /* if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);

        if( key->key() == Qt::Key_A ) { routate90(false); return true; }  // A
        if( key->key() == Qt::Key_S ) { routate90(true); return true; }  // S
        if( key->key() == Qt::Key_X ) {
            show_mode = 0; updatePreviewImage(); return true; }  // ' X '-RGB
        if( key->key() == Qt::Key_C ) {
            show_mode = 1; updatePreviewImage(); return true; }  // C
    }  // KeyPress
    if (event->type() == QEvent::MouseButtonPress) {
        if (object == addition) {
            cm = calcMode::cmAdd;
            titleEdit->setText("add");
        }
        if (object == subtraction) {
            cm = calcMode::cmSubt;
            titleEdit->setText("subtraction");
        }
        if (object == clipping) {
            cm = calcMode::cmClip;
            titleEdit->setText("clipping");
        }
    }  // MouseButtonPress */
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

void imageMask::setPixmapToPlot(QPixmap &pixmap)
{
    image_pixmap->topLeft->setCoords(0,pixmap.height());
    image_pixmap->bottomRight->setCoords(pixmap.width(),0);

    plot->xAxis->setRange(0,pixmap.width());
    plot->yAxis->setRange(0,pixmap.height());

    image_pixmap->setPixmap(pixmap);
    plot->replot();
}

void imageMask::updatePreviewImage()
{
    infoLabel->setText(getResultShowModesString());
    QPixmap pixmap;
    switch (show_mode) {
    case 0 :
        pixmap = *previewRGB;
        setButtonsEnabled(false);
        break;
    case 1 :
        pixmap = QPixmap::fromImage(result.img);
        setButtonsEnabled(true);
        break;
    }

    QMatrix rm;    rm.rotate(result.rotation);
    image_pixmap->setPixmap(pixmap.transformed(rm));
    image_pixmap->topLeft->setCoords(0,pixmap.height());
    image_pixmap->bottomRight->setCoords(pixmap.width(),0);
    plot->rescaleAxes();
    plot->replot();



}

void imageMask::clear()
{
    foreach(DropArea *da, pixmapLabelsVector) da->setNumA(-1);
    titleEdit->setText("Наименование");
    formulaEdit->setText("Формула");
    setDefaultRGB();

}

//void imageMask::cancel()
//{
//    formulaLabel->setText(result.title);
//    setButtonsEnabled(true);
//    show_mode = 1;
//    updatePreviewImage();
//}

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
        da->setNumA(da->getNumA());
    }  // foreach

    QMatrix rm;    rm.rotate(result.rotation);
    image_pixmap->setPixmap(QPixmap::fromImage(result.img).transformed(rm));
    plot->replot();
}

//void imageMask::plug(){}

//void imageMask::maskModify(int num)
//{
//    current_num = num;  show_mode = 1;  showModeLabel->setText(getResultShowModesString());
//    setButtonsEnabled(true);
//    switch (im) {
//    case imageMask::imNone : setMaskToMainPixmap(num);
//        break;
//    case imageMask::imAddition : doAddition(num);
//        updateResult(num);
//        break;
//    case imageMask::imSubtraction : doSubtraction(num);
//        updateResult(num);
//        break;
//    }
//}

void imageMask::doAddition(int num)
{
    QSize slice_size(result.mask[0].count(), result.mask.count());
    J09::maskRecordType *mask_added = imgHand->current_image()->getMask(num);

    QVector<int8_t> line(slice_size.width(), 1);
    QVector<QVector<int8_t> > buff(slice_size.height(), line);

    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++)
            if(result.mask[y][x] == 0 && mask_added->mask[y][x] == 0) buff[y][x] = 0;
    result.mask = buff;
}

void imageMask::doSubtraction(int num)
{
    QSize slice_size(result.mask[0].count(), result.mask.count());
    J09::maskRecordType *mask_added = imgHand->current_image()->getMask(num);

    QVector<int8_t> line(slice_size.width(), 0);
    QVector<QVector<int8_t> > buff(slice_size.height(), line);

    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++)
            if(result.mask[y][x] == 1 && mask_added->mask[y][x] == 0) buff[y][x] = 1;
    result.mask = buff;
}

void imageMask::updateResult(int num)
{
    J09::maskRecordType *m = imgHand->current_image()->getMask(num);
    QString firstTitle = result.title;
    result.title += formulaLabel->text() + m->title;
    result.formula = QString("%1: %2\n%3 %4").arg(firstTitle).arg(result.formula).arg(m->title).arg(m->formula);
    result.img = imgHand->current_image()->get_mask_image(result);
    show_mode = 1;
    updatePreviewImage();
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

void imageMask::setMaskToMainPixmap(int num)
{
    result = *imgHand->current_image()->getMask(num);
    QMatrix rm;    rm.rotate(result.rotation);
    QPixmap pixmap = QPixmap::fromImage(result.img).transformed(rm);
    image_pixmap->setPixmap(pixmap);
    image_pixmap->topLeft->setCoords(0,pixmap.height());
    image_pixmap->bottomRight->setCoords(pixmap.width(),0);
    plot->rescaleAxes();
    plot->replot();
    formulaLabel->setText(result.title);
}

void imageMask::setButtonsEnabled(bool enabled)
{
//    maskGroupBox->setEnabled(enabled);
    titleLabel->setEnabled(enabled);
    formulaLabel->setEnabled(enabled);
    formulaEdit->setEnabled(enabled);
    titleEdit->setEnabled(enabled);
    saveButton->setEnabled(enabled);
    clearButton->setEnabled(enabled);


    calculatorGroupBox->setEnabled(enabled);

//    additionButton->setEnabled(enabled);
//    subtractionButton->setEnabled(enabled);
//    cancelButton->setEnabled(true);
//    saveButton->setEnabled(enabled);
//    closeButton->setEnabled(enabled);
//    clearButton->setEnabled(enabled);

}

void imageMask::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(plot);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *act = menu->addAction("Сохранить изображение маски ...", this, &imageMask::saveMaskToPdfJpgPng);
    act->setIcon(QIcon(":/icons/theater.png"));
    act = menu->addAction("Сохранить маску в Excel *.csv файл ...", this, &imageMask::saveMaskToCsv);
    act->setIcon(QIcon(":/icons/theater.png"));

    menu->popup(plot->mapToGlobal(pos));
}

void imageMask::saveMaskToPdfJpgPng()
{

}

void imageMask::saveMaskToCsv()
{

}

void imageMask::execSlot(int num)
{
    if (num >= 0) {  // выбрано масочное изображение
        J09::maskRecordType *mask = imgHand->current_image()->getMask(num);

        QMatrix rm;    rm.rotate(mask->rotation);
        QPixmap pixmap = QPixmap::fromImage(mask->img).transformed(rm);

        setPixmapToPlot(pixmap);

        mask->formula_step_by_step.clear();
        mask->formula_step_by_step.append("fffff");
        mask->formula_step_by_step.append("wwwwwwwwwwwwwwwww");
        setButtonsEnabled(true);
        formulaEdit->clear();

        titleEdit->setText(mask->title);

        foreach(QString str, mask->formula_step_by_step)
            formulaEdit->append(str);
        return;
    }
}

void imageMask::loadMasksFromFile(QString fname)
{
    if (fname.isEmpty()) return;

    QFile datfile(fname);
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

void imageMask::saveMasksToFile(QString fname, QListWidget *mlw)
{
    if (fname.isEmpty()) return;
    int count = 0;
    for(int row=0; row < mlw->count(); row++)
        if (mlw->item(row)->checkState() == Qt::Checked) count++;
    if (count == 0) return;

    QFile datfile(fname);
    datfile.open(QIODevice::WriteOnly);
    QDataStream datstream( &datfile );

    datstream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    datstream.setByteOrder(QDataStream::LittleEndian);

    datstream << count;

    count = imgHand->current_image()->getMasksCount();

    int c = 0;
    for(int i=0; i<count; i++) {
        if (mlw->item(i)->checkState() != Qt::Checked) continue;
        J09::maskRecordType *m = imgHand->current_image()->getMask(i);
        datstream << m->checked << m->title << m->formula << m->invers;
        datstream << m->formula_step_by_step << m->img << m->brightness << m->rotation;
        datstream << m->mask;
        c++;
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
        setNumA(num);
    event->accept();
}

void DropArea::dropEvent(QDropEvent *event)
{
    empty = false;
    int cr = listWidget->currentRow();
    setNumA(cr);
    setBackgroundRole(QPalette::Dark);
    event->acceptProposedAction();
}

void DropArea::mousePressEvent(QMouseEvent *ev)
{
    emit exec(num);
}

void DropArea::setNumA(int n)
{
    num = n;  emit exec(num);

    if (n<0) {
        empty = true;
        clear(true);
        return;
    }
    empty = false;

    J09::maskRecordType *mask = imgHand->current_image()->getMask(num);

    QMatrix rm;    rm.rotate(mask->rotation);
    pixmap = QPixmap::fromImage(mask->img).transformed(rm);

    int h=this->height();
    int w=this->width();
    this->setPixmap(pixmap.scaled(w,h,Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    title->setText(mask->title);
    setToolTip(mask->formula);
}

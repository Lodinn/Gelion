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
    m_rotation = gsettings->main_rgb_rotate_start;
    mr_result.rotation = m_rotation;
    maskListWidget = lw;
    imgHand = ih;

    maskListWidget->setIconSize( defaultIconSize );
    maskListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    maskListWidget->setViewMode( QListWidget::IconMode );

    foreach(DropArea *da, pixmapLabelsVector) {
        da->listWidget = maskListWidget;
        da->imgHand = imgHand;
        da->rotation = m_rotation;
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
    setEnabledOfMainGroupsWidgets(true,true,false);
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
    setPixmapToPlot(defaultRGB,true);

    QGridLayout *titleGrid = new QGridLayout(maskGroupBox);
    plotLayout->addLayout(titleGrid, 3);

    titleLabel = new QLabel("Наименование", maskGroupBox);
    formulaLabel = new QLabel("Формула", maskGroupBox);

    titleEdit = new QLineEdit("Наименование", maskGroupBox);
    formulaEdit = new QTextEdit("Формула", maskGroupBox);
    formulaEdit->setReadOnly(true);

    infoLabel = new QLabel(getResultShowModesString(), maskGroupBox);
    saveButton = createButton(tr("Сохранить"), tr("Сохранить в списке масок"), SLOT(m_save()));
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

    // CONNECTIONS
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot, &QCustomPlot::customContextMenuRequested, this, &imageMask::contextMenuRequest);
    foreach(DropArea *da, pixmapLabelsVector)
        connect(da, &DropArea::exec, this, &imageMask::execSlot);
    connect(this, &imageMask::m_exec, this, &imageMask::execSlot);

    setEnabledOfMainGroupsWidgets(false,false,false);
//    return;
}

bool imageMask::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);

        if( key->key() == Qt::Key_X ) {
            show_mode = 0; m_exec(QPoint(0,1)); return true; }  // ' X '-RGB
        if( key->key() == Qt::Key_C ) {
            show_mode = 1; m_exec(QPoint(0,2)); return true; }  // ' C '-mask

//        if( key->key() == Qt::Key_A ) { m_exec(QPoint(0,3)); return true; }  // A
//        if( key->key() == Qt::Key_S ) { m_exec(QPoint(0,4)); return true; }  // S

    }  // KeyPress

    QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
    down_to_up = mouse->modifiers() == Qt::ControlModifier;

    if (event->type() == QEvent::MouseButtonPress) {
        if (object == addition) if (calculatorGroupBox->isEnabled()) m_exec(QPoint(-2,0));
        if (object == subtraction) if (calculatorGroupBox->isEnabled())  m_exec(QPoint(-3,0));
        if (object == clipping) if (calculatorGroupBox->isEnabled()) m_exec(QPoint(-4,0));
    }  // MouseButtonPress

    return QDockWidget::eventFilter(object, event);
}

void imageMask::setDefaultRGB()
{
    image_pixmap->topLeft->setCoords(0,defaultRGB.height());
    image_pixmap->bottomRight->setCoords(defaultRGB.width(),0);

    plot->xAxis->setRange(0,defaultRGB.width());
    plot->yAxis->setRange(0,defaultRGB.height());

    image_pixmap->setPixmap(defaultRGB);
    mr_result.img = defaultRGB.toImage();

    plot->replot();
}

void imageMask::setPixmapToPlot(QPixmap &pixmap, bool setRange)
{
    image_pixmap->topLeft->setCoords(0,pixmap.height());
    image_pixmap->bottomRight->setCoords(pixmap.width(),0);

    if (setRange) {
        plot->xAxis->setRange(0,pixmap.width());
        plot->yAxis->setRange(0,pixmap.height());
    }

    image_pixmap->setPixmap(pixmap);
    plot->replot();
}

void imageMask::updatePreviewImage()
{
    infoLabel->setText(getResultShowModesString());
    QPixmap pixmap;
    switch (show_mode) {
    case 0 : {
        pixmap = *previewRGB; qDebug() << "pixmap" << pixmap;
        setButtonsEnabled(false);
        bool gb_enabled = pixmapLabelsVector[0]->isEmptyA() || pixmapLabelsVector[1]->isEmptyA();
        calculatorGroupBox->setEnabled(!gb_enabled);
        break; }
    case 1 : {
        pixmap = QPixmap::fromImage(mr_result.img);
        setButtonsEnabled(true);
        bool gb_enabled = pixmapLabelsVector[0]->isEmptyA() || pixmapLabelsVector[1]->isEmptyA();
        calculatorGroupBox->setEnabled(!gb_enabled);
        break; }
    }

    QMatrix rm;    rm.rotate(mr_result.rotation);
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
    setEnabledOfMainGroupsWidgets(false,false,false);
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
        mr_result.rotation += 90.;
        if (mr_result.rotation > 360.) mr_result.rotation -= 360.;
    } else {
        mr_result.rotation -= 90.;
        if (mr_result.rotation < 0.) mr_result.rotation += 360.;
    }  // if

    updateMainPreviewWithIconsPreviw();
}

void imageMask::updatepixmapLabelsVector()
{
    foreach(DropArea *da, pixmapLabelsVector) {
        da->rotation = mr_result.rotation;
        da->setNumA(da->getNumA());
    }  // foreach
}

void imageMask::updateMainPreviewWithIconsPreviw()
{
    foreach(DropArea *da, pixmapLabelsVector) {
        da->rotation = mr_result.rotation;
        da->setNumA(da->getNumA());
    }  // foreach

    QMatrix rm;    rm.rotate(mr_result.rotation);
    image_pixmap->setPixmap(QPixmap::fromImage(mr_result.img).transformed(rm));
    plot->replot();
}

void imageMask::doAddition(int num)
{
    QSize slice_size(mr_result.mask[0].count(), mr_result.mask.count());
    J09::maskRecordType *mask_added = imgHand->current_image()->getMask(num);

    QVector<int8_t> line(slice_size.width(), 1);
    QVector<QVector<int8_t> > buff(slice_size.height(), line);

    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++)
            if(mr_result.mask[y][x] == 0 && mask_added->mask[y][x] == 0) buff[y][x] = 0;
    mr_result.mask = buff;
}

void imageMask::doSubtraction(int num)
{
    QSize slice_size(mr_result.mask[0].count(), mr_result.mask.count());
    J09::maskRecordType *mask_added = imgHand->current_image()->getMask(num);

    QVector<int8_t> line(slice_size.width(), 0);
    QVector<QVector<int8_t> > buff(slice_size.height(), line);

    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++)
            if(mr_result.mask[y][x] == 1 && mask_added->mask[y][x] == 0) buff[y][x] = 1;
    mr_result.mask = buff;
}

void imageMask::updateResult(int num)
{
    J09::maskRecordType *m = imgHand->current_image()->getMask(num);
    QString firstTitle = mr_result.title;
    mr_result.title += formulaLabel->text() + m->title;
    mr_result.formula = QString("%1: %2\n%3 %4").arg(firstTitle).arg(mr_result.formula).arg(m->title).arg(m->formula);
    mr_result.img = imgHand->current_image()->get_mask_image(mr_result);
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
    mr_result = *imgHand->current_image()->getMask(num);
    QMatrix rm;    rm.rotate(mr_result.rotation);
    QPixmap pixmap = QPixmap::fromImage(mr_result.img).transformed(rm);
    image_pixmap->setPixmap(pixmap);
    image_pixmap->topLeft->setCoords(0,pixmap.height());
    image_pixmap->bottomRight->setCoords(pixmap.width(),0);
    plot->rescaleAxes();
    plot->replot();
    formulaLabel->setText(mr_result.title);
}

void imageMask::setEnabledOfMainGroupsWidgets(bool e_left, bool e_right, bool e_save)
{
    titleLabel->setEnabled(e_left);  // = new QLabel("Наименование", maskGroupBox);
    formulaLabel->setEnabled(e_left); // = new QLabel("Формула", maskGroupBox);
    titleEdit->setEnabled(e_left); // = new QLineEdit("Наименование", maskGroupBox);
    formulaEdit->setEnabled(e_left); // = new QTextEdit("Формула", maskGroupBox);

    saveButton->setEnabled(e_save); // = createButton(tr("Сохранить"), tr("Сохранить в списке масок"), SLOT(m_save()));

    clearButton->setEnabled(e_left); // = createButton(tr("Очистить"), tr("Очистить изображение\nи иконки изображений-масок"), SLOT(clear()));

    if (e_right) {
        bool gb_enabled = pixmapLabelsVector[0]->isEmptyA() || pixmapLabelsVector[1]->isEmptyA();
        calculatorGroupBox->setEnabled(!gb_enabled);
    }
    else
        calculatorGroupBox->setEnabled(e_right);
}

void imageMask::setButtonsEnabled(bool enabled)
{
    titleLabel->setEnabled(enabled);
    formulaLabel->setEnabled(enabled);
    formulaEdit->setEnabled(enabled);
    titleEdit->setEnabled(enabled);
    saveButton->setEnabled(enabled);
    clearButton->setEnabled(enabled);
}

void imageMask::addition_calculate(J09::maskRecordType *mask1, J09::maskRecordType *mask2)
{
    // "1 + 1 = 1\n1 + 0 = 1\n0 + 1 = 1\n0 + 0 = 0"
    QSize slice_size(mask1->mask[0].count(), mask1->mask.count());

    QVector<int8_t> line(slice_size.width(), 1);
    QVector<QVector<int8_t> > buff(slice_size.height(), line);

    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++)
            if(mask1->mask[y][x] == 0 && mask2->mask[y][x] == 0) buff[y][x] = 0;
    mr_result.mask = buff;
    mr_result.img = get_mask_image(buff);
}

void imageMask::subtraction_calculate(J09::maskRecordType *mask1, J09::maskRecordType *mask2)
{
    // "1 - 1 = 0\n1 - 0 = 1\n0 - 1 = 0\n0 - 0 = 0"
    QSize slice_size(mask1->mask[0].count(), mask1->mask.count());

    QVector<int8_t> line(slice_size.width(), 0);
    QVector<QVector<int8_t> > buff(slice_size.height(), line);

    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++)
            if(mask1->mask[y][x] == 1 && mask2->mask[y][x] == 0) buff[y][x] = 1;
    mr_result.mask = buff;
    mr_result.img = get_mask_image(buff);
}

void imageMask::clipping_calculate(J09::maskRecordType *mask1, J09::maskRecordType *mask2)
{
    // "1 cl 1 = 1\n1 cl 0 = 0\n0 cl 1 = 0\n0 cl 0 = 0"
    QSize slice_size(mask1->mask[0].count(), mask1->mask.count());

    QVector<int8_t> line(slice_size.width(), 0);
    QVector<QVector<int8_t> > buff(slice_size.height(), line);

    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++)
            if(mask1->mask[y][x] == 1 && mask2->mask[y][x] == 1) buff[y][x] = 1;
    mr_result.mask = buff;
    mr_result.img = get_mask_image(buff);
}

QImage imageMask::get_mask_image(QVector<QVector<int8_t> > &m)
{
    if(m.isEmpty()) {
      qDebug() << "CRITICAL! AN EMPTY SLICE RETRIEVED WHILE CONSTRUCTING THE INDEX IMAGE";
      return QImage();
    }
    QSize slice_size(m[0].count(), m.count());
    QImage mask_img(slice_size, QImage::Format_Mono);

    mask_img.fill(1);
    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++)
            if (m[y][x] == 0) mask_img.setPixel(x, y, 0);

    return mask_img;
}

/*if (down_to_up) {
        plv1 = pixmapLabelsVector[1]; plv2 = pixmapLabelsVector[0];
    } else {
        plv1 = pixmapLabelsVector[0]; plv2 = pixmapLabelsVector[1];
    }*/

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

void imageMask::execSlot(QPoint p)
{
    if(p.x() == -1) return;

    if (p.x() >= 0 && p.y() == 0) {  // выбрано масочное изображение

        setEnabledOfMainGroupsWidgets(true,true,false);
        show_mode = 1;  infoLabel->setText(getResultShowModesString());
        J09::maskRecordType *mask = imgHand->current_image()->getMask(p.x());

        QMatrix rm;    rm.rotate(mask->rotation);
        QPixmap pixmap = QPixmap::fromImage(mask->img).transformed(rm);

        mr_result.img = mask->img;
        mr_result.rotation = mask->rotation;
        mr_result.title = mask->title;
        mr_result.formula_step_by_step = mask->formula_step_by_step;

        setPixmapToPlot(pixmap,true);

        titleEdit->setText(mask->title);

        formulaEdit->clear();
        foreach(QString str, mask->formula_step_by_step)
            formulaEdit->append(str);

        return;
    }

    if (p.x() == 0 && p.y() > 0) {  // выбран специальный режим
        switch (p.y()) {
        case 1 : {  show_mode = 0;  infoLabel->setText(getResultShowModesString());     // ' X '-RGB
            setEnabledOfMainGroupsWidgets(false,false,false);
            QMatrix rm;    rm.rotate(mr_result.rotation);
            QPixmap pixmap = previewRGB->transformed(rm);
            setPixmapToPlot(pixmap,false);
            titleEdit->setText("RGB");  formulaEdit->clear();
            break; }
        case 2 : {  show_mode = 1;  infoLabel->setText(getResultShowModesString());     // ' C '-mask
            setEnabledOfMainGroupsWidgets(true,true,false);
            QMatrix rm;    rm.rotate(mr_result.rotation);
            QPixmap pixmap = QPixmap::fromImage(mr_result.img).transformed(rm);
            setPixmapToPlot(pixmap,false);
            titleEdit->setText(mr_result.title);
            formulaEdit->clear();
            foreach(QString str, mr_result.formula_step_by_step)
                formulaEdit->append(str);
            break; }
        case 3 : {  // А по часовой
            mr_result.rotation += 90.;  qDebug() << "3 mr_result.rotation" << mr_result.rotation;
            if (mr_result.rotation > 360.) mr_result.rotation -= 360.;
            QMatrix rm;    rm.rotate(mr_result.rotation);
            QPixmap pixmap;
            if (show_mode == 0) pixmap = previewRGB->transformed(rm);
            else pixmap = QPixmap::fromImage(mr_result.img).transformed(rm);
            setPixmapToPlot(pixmap,false);
//            updatepixmapLabelsVector();
            break; }
        case 4 : {  // S против часовой
            mr_result.rotation -= 90.;  qDebug() << "4 mr_result.rotation" << mr_result.rotation;
            if (mr_result.rotation < .0) mr_result.rotation += 360.;
            QMatrix rm;    rm.rotate(mr_result.rotation);
            QPixmap pixmap;
            if (show_mode == 0) pixmap = previewRGB->transformed(rm);
            else pixmap = QPixmap::fromImage(mr_result.img).transformed(rm);
            setPixmapToPlot(pixmap,false);
//            updatepixmapLabelsVector();
            break; }
        }

        return;
    }

// выбран расчетный метод
    show_mode = 1;  infoLabel->setText(getResultShowModesString());                     // ' C '-mask
    setEnabledOfMainGroupsWidgets(true,true,true);

    DropArea *plv1;
    DropArea *plv2;
    if (down_to_up) {
        plv1 = pixmapLabelsVector[1]; plv2 = pixmapLabelsVector[0];
    } else {
        plv1 = pixmapLabelsVector[0]; plv2 = pixmapLabelsVector[1];
    }

    QString t1 = plv1->title->text();
    QString t2 = plv2->title->text();

    J09::maskRecordType *mask1 = imgHand->current_image()->getMask(plv1->getNumA());
    J09::maskRecordType *mask2 = imgHand->current_image()->getMask(plv2->getNumA());

    QStringList fsbs1 = mask1->formula_step_by_step;
    QStringList fsbs2 = mask2->formula_step_by_step;

    // get basic for one
    /*int b_count_1 = fsbs1.count() - 1;
    QStringList b_s_list_1;  for (int i=0; i < b_count_1; i++) b_s_list_1.append(fsbs1[i]);

    // get basic for two
    int b_count_2 = fsbs2.count() - 1;
    QStringList b_s_list_2;  for (int i=0; i < b_count_2; i++) b_s_list_2.append(fsbs2[i]);

    QString formula1 = QString("%1::%2").arg(t1).arg(fsbs1[b_count_1]);
    QString formula2 = QString("%1::%2").arg(t2).arg(fsbs2[b_count_2]);

    QStringList result;
    result.append(b_s_list_1);   result.append(b_s_list_2);
    result.append(formula1);   result.append(formula2);*/

    QStringList result;
    int b_count_1 = fsbs1.count() - 1;
    QStringList b_s_list_1;  for (int i=0; i < b_count_1; i++)
        if(result.indexOf(fsbs1[i]) == -1) result.append(fsbs1[i]);
    int b_count_2 = fsbs2.count() - 1;
    QStringList b_s_list_2;  for (int i=0; i < b_count_2; i++)
        if(result.indexOf(fsbs2[i]) == -1) result.append(fsbs2[i]);
    QString formula1 = QString("%1::%2").arg(t1).arg(fsbs1[b_count_1]);
    QString formula2 = QString("%1::%2").arg(t2).arg(fsbs2[b_count_2]);
    result.append(formula1);   result.append(formula2);

    switch (p.x()) {
    case -2 : {                     // addition                      !сложение
        QString res_title = QString("%1 + %2").arg(t1).arg(t2);
        titleEdit->setText(res_title);
        result.append(QString("$%1$ addition $%2$").arg(t1).arg(t2));
        addition_calculate(mask1, mask2);      // сложение
        break; }
    case -3 : {                     // subtraction                   !вычитание
        QString res_title = QString("%1 - %2").arg(t1).arg(t2);
        titleEdit->setText(res_title);
        result.append(QString("$%1$ subtraction $%2$").arg(t1).arg(t2));
        subtraction_calculate(mask1, mask2);   // вычитание
        break; }
    case -4 : {                     // clipping                       !отсечение
        QString res_title = QString("%1 cl %2").arg(t1).arg(t2);
        titleEdit->setText(res_title);
        result.append(QString("$%1$ clipping $%2$").arg(t1).arg(t2));
        clipping_calculate(mask1, mask2);      // отсечение
        break; }
    }
    // отображение результата
    QMatrix rm;    rm.rotate(mr_result.rotation);
    QPixmap pixmap = QPixmap::fromImage(mr_result.img).transformed(rm);
    setPixmapToPlot(pixmap,false);

    formulaEdit->clear();
    foreach(QString str, result) formulaEdit->append(str);

    mr_result.title = titleEdit->text();
    mr_result.formula_step_by_step = result;
    mr_result.formula.clear();
    foreach(QString str, result) {
        int num = result.indexOf(str);
        if (num == result.count() - 1)
            mr_result.formula.append(str);
        else
            mr_result.formula.append(str + '\n');
    }
}

void imageMask::m_save()
{
    J09::maskRecordType *mask_appended = new J09::maskRecordType;
    *mask_appended = mr_result;
    mask_appended->checked = true;
    mask_appended->title = titleEdit->text();
    emit appendMask(mask_appended);
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
    emit exec(QPoint(num,0));
}

void DropArea::setNumA(int n)
{
    num = n;  emit exec(QPoint(num,0));

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

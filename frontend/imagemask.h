#ifndef IMAGEMASK_H
#define IMAGEMASK_H

#include "qcustomplot.h"

#include <ImageHandler.h>
#include <QDockWidget>
#include <QListWidget>

class Button : public QToolButton
{
    Q_OBJECT
public:
    explicit Button(const QString &text, QWidget *parent = 0);
};

class DropArea : public QLabel
{
    Q_OBJECT

public:
    DropArea(QWidget *parent = 0);
    QLabel *title;
    QListWidget *listWidget;
    ImageHandler *imgHand = nullptr;
    QCustomPlot *plot;
    QCPItemPixmap *image_pixmap;
    QPixmap pixmap;
    QVector<QVector<int8_t> > *result;  // ссылка на массив для хранения результатов операций
    void setNum(int n);
    int getNum() { return num; }
    double rotation = .0;
public slots:
    void clear(bool full);
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
// Класс QDragEnterEvent предоставляет событие, которое отправляется виджету,
// когда в него входит действие перетаскивания.
// Класс QDragMoveEvent предоставляет событие, которое отправляется
// во время выполнения действия перетаскивания.
// Класс QDragLeaveEvent предоставляет событие, которое отправляется виджету,
// когда действие перетаскивания покидает его.
// Класс QDropEvent предоставляет событие, которое отправляется
// после завершения действия перетаскивания.
private:
   bool empty = true;
   int num;
   QString defTitleString = QString("Наименование");
signals:
   void exec(int num);
};

class imageMask : public QDockWidget
{
    Q_OBJECT
public:
    imageMask(QWidget * parent = nullptr);
    void setLWidgetAndIHandler(QListWidget *lw, ImageHandler *ih);
    QListWidgetItem *createMaskWidgetItem(J09::maskRecordType *am, QImage &img);
    void set4MasksToForm();
    J09::globalSettingsType *gsettings;
    void saveMasksToFile(QString data_file_name);
    void loadMasksFromFile(QString data_file_name);
    void updateMaskListWidget();
    void setPreviewPixmap(QPixmap &mainRGB);
private:
    QString defMaskFileDataName = "masks.dat";
    enum inputMode { imNone, imAddition, imSubtraction };
    inputMode im = imageMask::imNone;
    int show_mode = 1;  // 1 - mask 0 - RGB
    int current_num = -1;
    enum calcMode { cmAdd, cmSubt, cmClip };
    calcMode cm = calcMode::cmAdd;

    void setupUi();
    bool eventFilter(QObject *object, QEvent *event);
    QCustomPlot *plot = nullptr;
    QCPItemPixmap *image_pixmap = nullptr;
    QPixmap *previewRGB;
    QListWidget *maskListWidget = nullptr;
    ImageHandler *imgHand = nullptr;
    QSize defaultIconSize = QSize(32+64,32+64);
    QPixmap defaultRGB = QPixmap(512,512);
    void setDefaultRGB();
    void setRGBtoPlot(QPixmap &pixmap);
    void updatePreviewImage();
    J09::maskRecordType result;  // массив для хранения результатов операций

    QString getResultShowModesString();
    QStringList resultShowModesList = QStringList() <<  "RGB" << "Маска";
    QString resultShowModesStr = "' X '-RGB  ' C '-маска  ' A '-по часовой  ' S '-против часовой ( <b>%1</b> )";
    QLabel *showModeLabel = new QLabel;
    Button *createButton(const QString &text, const QString &tooltip, const char *member);
    QLabel *createHeaderLabel(const QString &text);
    DropArea *createPixmapLabel();
    enum { mask_row_count = 2, mask_col_count = 2 };
    DropArea *pixmapLabels[mask_col_count][mask_row_count];
    QVector<DropArea *> pixmapLabelsVector;
    QVector<QRadioButton *> calculationButtons;

    QString defTitleString = QString("Наименование");
    double rotation = .0;
    void routate90(bool clockwise);
    void updateMainPreviewWithIconsPreviw();
    void doAddition(int num);
    void doSubtraction(int num);
    void updateResult(int num);
    QString getMaskDataFileName(QString data_file_name);
    void setMaskToMainPixmap(int num);
// BUTTONS
    void setButtonsEnabled(bool enabled);
    Button *additionButton;
    Button *subtractionButton;
    Button *cancelButton;
    Button *saveButton;
    Button *closeButton;
    Button *clearButton;
    QRadioButton *addition;
    QRadioButton *subtraction;
    QRadioButton *clipping;
    QLabel *titleLabel;
    QLabel *formulaLabel;
    QTextEdit *formulaEdit;
    QLineEdit *titleEdit;
private slots:
    void plug();  // заглушка
    void maskModify(int num);
    void clear();
    void cancel();
};

#endif // IMAGEMASK_H

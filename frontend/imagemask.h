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
    QCustomPlot *plot;
    QCPItemPixmap *image_pixmap;
    QPixmap *image;
public slots:
    void clear();
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

};

class imageMask : public QDockWidget
{
    Q_OBJECT
public:
    imageMask(QWidget * parent = nullptr);
    void setPreviewPixmap(QPixmap &mainRGB);
    void setLWidgetAndIHandler(QListWidget *lw, ImageHandler *ih);
    QListWidgetItem *createMaskWidgetItem(const QString &atext, const QString &atoolTip, const QIcon &aicon);
private:
    void setupUi();
    QCustomPlot *plot = nullptr;
    QCPItemPixmap *image_pixmap = nullptr;
    QPixmap *previewRGB = nullptr;
    QListWidget *maskListWidget = nullptr;
    ImageHandler *imgHand = nullptr;
    QSize defaultIconSize = QSize(64,64);
    QPixmap defaultRGB = QPixmap(512,512);
    void setDefaultRGB();
    QString getResultShowModesString();
    int show_mode = 1;
    QStringList resultShowModesList = QStringList() <<  "RGB" << "Маска";
    QString resultShowModesStr = "' X '-RGB  ' C '-маска  ' A '-по часовой  ' S '-против часовой ( <b>%1</b> )";
    QLabel *formulaLabel = new QLabel;
    QLabel *showModeLabel = new QLabel;
    Button *createButton(const QString &text, const QString &tooltip, const char *member);
    QLabel *createHeaderLabel(const QString &text);
    DropArea *createPixmapLabel();
private slots:
    void plug();  // заглушка
};

#endif // IMAGEMASK_H

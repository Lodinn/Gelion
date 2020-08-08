#ifndef IMAGEMASK_H
#define IMAGEMASK_H

#include "qcustomplot.h"

#include <ImageHandler.h>
#include <QDockWidget>
#include <QListWidget>

class imageMask : public QDockWidget
{
public:
    imageMask(QWidget * parent = nullptr);
    void setLWidgetAndIHandler(QListWidget *lw, ImageHandler *ih);
private:
    void setupUi();
    QCustomPlot *previewPlot = nullptr;
    QListWidget *maskListWidget = nullptr;
    ImageHandler *imgHand = nullptr;
    QSize defaultIconSize = QSize(64,64);
};

#endif // IMAGEMASK_H

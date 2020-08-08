#include "imagemask.h"

imageMask::imageMask(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi();
    move(750,500); resize(700,400);

}

void imageMask::setLWidgetAndIHandler(QListWidget *lw, ImageHandler *ih)
{
    maskListWidget = lw;
    imgHand = ih;

    maskListWidget->setIconSize( defaultIconSize );
    maskListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    maskListWidget->setViewMode( QListWidget::IconMode );

}

void imageMask::setupUi()
{
    setObjectName("mask");
    setWindowTitle("Калькулятор масочных изображений");
    setAllowedAreas(nullptr);
    setFloating(true);
}

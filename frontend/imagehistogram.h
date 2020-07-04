#ifndef IMAGEHISTOGRAM_H
#define IMAGEHISTOGRAM_H

#include "qcustomplot.h"
#include "backend/SpectralImage.h"

#include <QDockWidget>

class imageHistogram : public QDockWidget
{
    Q_OBJECT
public:
    imageHistogram(QWidget * parent = nullptr);
    void updateData(QString name, QString formula, QVector<QVector<double> > img,
                    J09::histogramType &hg);
    void setPreviewPixmap(QPixmap &mainRGB);
private:
    void  calculateHistogram(bool full);
    bool eventFilter(QObject *object, QEvent *event);
    void setupUi();
    void setupConnections();
    QCustomPlot *plot, *previewPlot;
    QCPTextElement *title;
    QGroupBox *bottomGroupBox;
    QGridLayout *gridLayout;
    QSlider *sliderBrightness;
    QSlider *sliderLeftColorEdge;
    QSlider *sliderRightColorEdge;
    QCheckBox *previewImage;
    QLabel *labelBrightness;
    QLabel *labelLeftBlue;
    QLabel *labelRightRed;
    QPushButton *buttonAxisRescale;
//    QPushButton *button90routate;
    QVBoxLayout *vertLayout;
    QLabel *labelPreview;
    QHBoxLayout *horzLayout;
    QCPGraph *packet_lower, *packet_upper;
    QCPItemBracket *bracket;
    QCPItemText *bracketText;
    QCPGraph *packet_left, *packet_center, *packet_right;
    double bracketTextValue = 7.;
    J09::histogramType *h_data = nullptr;
    void setHistogramToSliders();
    void getHistogramFromSliders();
    void updatePreviewImage();
    int histogramPlotTrigger = 0;
    bool histogramPlotPAN = false;
    QVector<QVector<double> > slice;
    QSize main_size;
    QImage get_index_rgb_ext(QVector<QVector<double> > &slice, bool colorized);
    QRgb get_rainbow_RGB(double Wavelength);
    void setHistogramToBracked();
    QPixmap changeBrightnessPixmap(QImage &img, double brightness);
    double bracketTextIndent = .9;
    QCPGraph *graph;
    QPixmap *previewRGB;
    QCPItemPixmap *image_pixmap;
    void setupPreviewPlot();  // настройка параметров просмотра
    void updatePreviewData();  // загрузка параметров изображения
private slots:
    void brightnessChanged();
    void leftColorChanged();
    void rightColorChanged();
    void mousePress(QMouseEvent *event);
    void mouseMove(QMouseEvent *event);
    void mouseRelease(QMouseEvent *event);
    void histogramPreview();
    void axisRescale();
    void routate90(bool clockwise);
    void contextMenuRequest(QPoint pos);
    void savePlotToPdfJpgPng();
    void saveIndexToPdfJpgPng();


};

#endif // IMAGEHISTOGRAM_H

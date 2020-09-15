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
    void updateData(QString data_file_name, slice_magic *sm);
    void setPreviewPixmap(QPixmap &mainRGB);
private:
    void calculateHistogram(bool full);
    bool eventFilter(QObject *object, QEvent *event);
    void setupUi();
    void setupConnections();
    void setupSlidersConnections();
    void disconnectSlidersConnections();
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
    QPushButton *buttonMaskSave;
    QVBoxLayout *vertLayout;
    QLabel *labelPreview;
    QLabel *labelPreviewStatus = new QLabel();
    QStringList previewStatusList = QStringList() << "Индекс" << "RGB" << "Маска" << "Инв.маска";
    QString previewStatusStr = "' Z '-индекс ' X '-RGB ' C '-маска ' V '-инв.маска ' A '-по часовой ' S '-против часовой ( <b>%1</b> )";
    QString getPreviewStatusString();
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
    slice_magic *sm_buff;
    QSize main_size;
    QImage get_index_rgb_ext(QVector<QVector<double> > &slice, bool colorized);
    QImage get_mask_image(QVector<QVector<double> > &slice, bool inversion);
    QRgb get_rainbow_RGB(double Wavelength);
    void setHistogramToBracked();
    QPixmap changeBrightnessPixmap(QImage &img, double brightness);
    double bracketTextIndent = .84;
    QCPGraph *graph;
    QPixmap *previewRGB;
    QCPItemPixmap *image_pixmap;
    void setupPreviewPlot();  // настройка параметров просмотра
    void updatePreviewData();  // загрузка параметров изображения
    QString f_data_file_name, f_title, f_formula;
    QString getMaskFormula();
    QString getMaskTitle();
    slice_magic *mask_appended;
    QString getWritableLocation();
    QVector<QVector<int8_t> > calculateMask(bool inv);  // маска
    QImage get_mask_image(QVector<QVector<int8_t> > mask);
    QGroupBox *centralGroupBox;
    QVBoxLayout *centralGBLayout;
    QLineEdit *title_of_mask;
    QLineEdit *formula_of_mask;
private slots:
    void brightnessChanged();
    void leftColorChanged();
    void rightColorChanged();
    void mousePress(QMouseEvent *event);
    void mouseMove(QMouseEvent *event);
    void mouseRelease(QMouseEvent *event);
    void histogramPreview();
    void axisRescale();
    void maskSave();
    void routate90(bool clockwise);
    void contextMenuRequest(QPoint pos);
    void savePlotToPdfJpgPng();
    void saveHistogramToCsv();
    void saveIndexToPdfJpgPng();
    void saveMaskToPdfJpgPng();
    void saveInvMaskToPdfJpgPng();
    void saveRGBToPdfJpgPng();
    void saveIndexToCsv();
    void saveMaskToCsv();
    void saveInvMaskToCsv();
signals:
    void appendMask(slice_magic *sm);
};

#endif // IMAGEHISTOGRAM_H

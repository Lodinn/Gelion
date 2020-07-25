#ifndef IMAGESPECTRAL_H
#define IMAGESPECTRAL_H

#include "frontend/qcustomplot.h"
#include "frontend/zgraph.h"
#include "backend/SpectralImage.h"

#include <QDockWidget>

class SpectralPlot : public QDockWidget
{
    Q_OBJECT
public:
    SpectralPlot(QWidget * parent = nullptr);
    void updateData(QString data_file_name, QList<zGraph *> list, bool rescale);
    void updateDataOneProfile(zGraph *item, int num);
    int getGraphCount() { return plot->graphCount()- mRainbow; }
    QCheckBox *rainbowCheckBox;
private:
    QCustomPlot *plot;
    QString f_data_file_name;
    QList<zGraph *> graph_list;
    QVector<J09::plotStyle> plot_styles_cycle;
    bool eventFilter(QObject *object, QEvent *event);
    void setupUi();
    void setupConnections();
    QCPItemTracer *tracer, *tracer_tolltip;
    QGroupBox *bottomGroupBox;
    QGridLayout *gridLayout;
    QTextEdit *textEdit;
    QPushButton *axisResizeButton;
    QStatusBar *statusBar;

    QVBoxLayout *dockLayout;
    void spectralSetAllRange();
    double sXmin, sXmax;
    double sYmin, sYmax;
    int tracerIndex;
    QStringList getGraphClickedStrings(int dataIndex, QString name, double keyValue, double dataValue);
    int mRainbow;
    void updateDataRainbow(bool r_show);
    void drawRainbowSpectralRanges();
    void updateRainbowSpectralRanges();
    void MouseOverPlotHeight(QMouseEvent *event);
    void DisplayCurveData(QMouseEvent *event, QCustomPlot *curPlot, QString strNameX, QString strNameY);
    QString getWritableLocation();
    void setRainbowSpectralRanges();
    void setStdVevProfiles();  // профили стандартного отклонения
    int rainbowTransparency = 50;
    int rainbowNum;
    QVector<QPointF> x_rainbow;  // координаты заливки спектральных диапазонов
    QVector<QColor> rgb_rainbow;  // цвет заливки спектральных диапазонов
    double rainbowUp = 25.;
    void setupRainbowCoordinates();
    void createRainbow();

private slots:
    void selectionChanged();
    void mousePress(QMouseEvent *event);
    void mouseWheel();
    void mouseMove(QMouseEvent *event);
    void graphClicked(QCPAbstractPlottable*,int);
    void rainbowShow(bool checked);
    void contextMenuRequest(QPoint pos);
    void savePlotToPdfJpgPng();
    void savePlotToCsv();
    void savePlotToRoi();
    void updateRainbow();

};

#endif // IMAGESPECTRAL_H

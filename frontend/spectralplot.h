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
    void setRainbowSpectralRanges();
    QStringList getGraphClickedStrings(int dataIndex, QString name, double keyValue, double dataValue);
    int mRainbow;
    void updateDataRainbow(bool r_show);
    void MouseOverPlotHeight(QMouseEvent *event);
    void DisplayCurveData(QMouseEvent *event, QCustomPlot *curPlot, QString strNameX, QString strNameY);
    QString getWritableLocation();

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
};

#endif // IMAGESPECTRAL_H

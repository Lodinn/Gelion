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
    void updateData(QList<zGraph *> list, bool rescale);
    void updateDataOneProfile(zGraph *item, int num);
    int getGraphCount() { return plot->graphCount()- mRainbow; }
private:
    QCustomPlot *plot;
    QList<zGraph *> graph_list;
    QVector<J09::plotStyle> plot_styles_cycle;
    bool eventFilter(QObject *object, QEvent *event);
    void setupUi();
    void setupConnections();
    QCPItemTracer *tracer;
    QGroupBox *bottomGroupBox;
    QGridLayout *gridLayout;
    QTextEdit *textEdit;
    QPushButton *axisResizeButton;
    QCheckBox *rainbowCheckBox;
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
private slots:
    void selectionChanged();
    void mousePress(QMouseEvent *event);
    void mouseWheel();
    void mouseMove(QMouseEvent *event);
    void graphClicked(QCPAbstractPlottable*,int);
    void rainbowShow(bool checked);
};

#endif // IMAGESPECTRAL_H

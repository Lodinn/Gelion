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
    void updateData(QString data_file_name, QList<zGraph *> list);
    void updateDataExt(QString data_file_name, QList<zGraph *> list, zGraph *item);
    void updateDataOneProfile(zGraph *item);
    int getGraphCount() { return plot->graphCount()- mRainbow; }
    QCheckBox *rainbowCheckBox = new QCheckBox("Заливка спектральных диапазонов");
    QCheckBox *deviationCheckBox = new QCheckBox("Дисперсия ( 95% Ci )");
    J09::globalSettingsType *gsettings;
    void setDefaultState();  // настройки окна по умолчанию
    ImageHandler *imgHand;
private:
    QCustomPlot *plot;
    QString f_data_file_name;
    QList<zGraph *> graph_list;
    QList<zGraph *> get_visible_graph_list(QList<zGraph *> list);
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
    int deviationTransparency = 75;
    double rainbowUp = 25.;
    int rainbowNum;
    QVector<QPointF> x_rainbow;  // координаты заливки спектральных диапазонов
    QVector<QColor> rgb_rainbow;  // цвет заливки спектральных диапазонов
    void setupRainbowCoordinates();
    void createRainbow();
    void createMainGraphs();
    void updateRanges();
    void createAdditinalGraphs();
    void updateRainbow();
    QString rainbowUniqueStr = "rainbow$%^&*()";
    QString devLowerUniqueStr = "devLower$%^&*()";
    QString devUpperUniqueStr = "devUpper$%^&*()";
    void updateDataExtBasic();
    void updateAdditinalGraphs();
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
    void updateRainbowSlot();
    void updateMainGraphRanges();
    void updateAdditinalGraphsSlot();
};

#endif // IMAGESPECTRAL_H

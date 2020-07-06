#ifndef ZGRAPH_H
#define ZGRAPH_H

#include "qcustomplot.h"
#include "zgraphparamdlg.h"
#include "ui_zgraphparamdlg.h"

#include <QDockWidget>
#include <QFont>
#include <QGraphicsItem>
#include <QPen>

QT_BEGIN_NAMESPACE
namespace ZG {
struct histogramType {
    double min = INT_MAX;
    double max = INT_MIN;
    double sum = .0;
    int hcount = 256;
    QVector<double > vx, vy;  // статистика / совместимлсть с qCustomPlot
    double lower, upper, sum_of_part;
    double wl380 = 380.;
    double wl781 = 781.;
    double brightness = 1.;
    double ___plotmouse = .005;
    double ___sbrightness = .3;
    bool imgPreview = true;
    bool colorized = true;
};
}
QT_END_NAMESPACE

//  Семейство графических объектов серии z ... область интереса "ОИ"
// - точка             Point         1
// - прямоугольник     Rect          2	с возможностью поворота
// - эллипс            Ellipse       3	с возможностью поворота
// - полилиния         Polyline      4
// - область(полигон)  Polygon       5

class zGraph : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit zGraph();                              // базовый объект
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void setTitle(QString title);
    QString getTitle() const;
    void setFont(QString family, int pointSize, int weight, bool italic);
    virtual QRectF getTitleRectangle() const = 0;
    virtual void updateBoundingRect() = 0;

    QDockWidget *dockw = nullptr;
    QCustomPlot *plot = nullptr;
    void setContextMenuConnection();

    QPoint dockwpos;
    QListWidgetItem *listwidget = nullptr;
    virtual bool pointIn(QPointF point) = 0;
    virtual QPolygonF getTitlePolygon() = 0;
    virtual QStringList getSettings(int num) = 0;
    char setsep = '#';
    QIcon aicon;
    QString typeString = "";
    void setParamsToDialog(zgraphParamDlg *dlg);
    void getParamsFromDialog(zgraphParamDlg *dlg);
    double getRotationFromCoords(QPointF p1, QPointF p2);
    QVector<QPointF > profile;
    double sigma = .0;
    QVector<double > pup, plw;
    void calculateProfileWithSigma(QVector<double> x, QVector<double> y, QVector<double> yup, QVector<double> ylw);
    int transparency = 50;
signals:
     void mouseMove();
     void mouseRelease();
     void itemChange();
     void itemDelete(zGraph *);
     void changeParamsFromDialog(zGraph *);
private slots:
     void contextMenuRequest(QPoint pos);
     void savePlotToPdfJpgPng();
     void savePlotToCsv();
     void savePlotToRoi();
     void plotRescaleAxes();
protected:
     int fRectIndent = 12;
     QPointF fcenterPoint;
     QRectF fbrect;
     QFont ffont = QFont("Helvetica", fRectIndent*2/3, -1, true);
     QRectF ftitleRect;
     QPolygonF ftitlePolygon;
     QPen selectedPen = QPen(Qt::green, 1, Qt::DashLine);
     QPen basePen = QPen(Qt::green, 1, Qt::SolidLine);
     QBrush selectedBrush = QBrush(Qt::yellow,Qt::Dense7Pattern);
     QBrush baseBrush = QBrush(Qt::lightGray,Qt::Dense7Pattern);
     qreal fOpacity = 0.99;
     QPen titleRectPen = QPen(Qt::black, 1, Qt::SolidLine);
     QBrush titleRectBrush = QBrush(Qt::white);
     QBrush titleRectBrushSelection = QBrush(Qt::cyan);
     QPen titleTextPen = QPen(Qt::darkRed, 1, Qt::SolidLine);
protected:
     void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
     void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
     QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
     void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
     QPointF getRectPoint2(QPointF p1, qreal w, qreal rot);
private:
};

class zPoint : public zGraph
{
    Q_OBJECT
public:
    enum { Type = UserType + 1 };  // тип объекта - точка             Point         1
    int type() const override { return Type; }
public:
    explicit zPoint(const QPointF point);
    QRectF getTitleRectangle() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    virtual void updateBoundingRect();
    virtual bool pointIn(QPointF point);
    virtual QPolygonF getTitlePolygon();
    virtual QStringList getSettings(int num);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;
};

class zRect : public zGraph
{
    Q_OBJECT
public:
    enum { Type = UserType + 2 };  // тип объекта - прямоугольник Rect 2	с возможностью поворота
    int type() const override { return Type; }
public:
    explicit zRect(const QPointF point, qreal scale, qreal rotate);
    QRectF getTitleRectangle() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    virtual void updateBoundingRect();
    virtual bool pointIn(QPointF point);
    virtual QPolygonF getTitlePolygon();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;
    QSizeF frectSize = QSize(150,50);
    qreal fdefaultHeigth = 24;
    virtual QStringList getSettings(int num);
protected:
    QPointF getCenterPoint();
};

class zEllipse : public zGraph
{
    Q_OBJECT
public:
    enum { Type = UserType + 3 };  // тип объекта - эллипс Ellipse  3	с возможностью поворота
    int type() const override { return Type; }
public:
    explicit zEllipse(const QPointF point, qreal scale, qreal rotate);
    QRectF getTitleRectangle() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    virtual void updateBoundingRect();
    virtual bool pointIn(QPointF point);
    virtual QPolygonF getTitlePolygon();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;
    QSizeF frectSize = QSize(150,50);
    qreal fdefaultHeigth = 24;
    virtual QStringList getSettings(int num);
protected:
    QPointF getCenterPoint();
};

class zPolyline : public zGraph
{
    Q_OBJECT
public:
    enum { Type = UserType + 4 };  // тип объекта - полилиния Polyline  4
    int type() const override { return Type; }
public:
    explicit zPolyline(qreal scale, qreal rotate);
    QRectF getTitleRectangle() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    virtual void updateBoundingRect();
    virtual bool pointIn(QPointF point);
    virtual QPolygonF getTitlePolygon();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;
    void addPoint(QPoint point);
    QPolygon fpolygon;
    virtual QStringList getSettings(int num);

protected:
    QPointF getCenterPoint();
};

class zPolygon : public zGraph
{
    Q_OBJECT
public:
    enum { Type = UserType + 5 };  // тип объекта - область(полигон) Polygon 5
    int type() const override { return Type; }
public:
    explicit zPolygon(qreal scale, qreal rotate);
    QRectF getTitleRectangle() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    virtual void updateBoundingRect();
    virtual bool pointIn(QPointF point);
    virtual QPolygonF getTitlePolygon();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;
    void addPoint(QPoint point);
    QPolygon fpolygon;
    virtual QStringList getSettings(int num);
protected:
    QPointF getCenterPoint();
 };

class zContourRect : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 77 };  // тип объекта - прямоугольники выделения и редактирования
    int type() const override { return Type; }
public:
    explicit zContourRect(QPointF point);
    ~zContourRect();
signals:
    void mousePress();
    void mouseMove(int num, QPointF point);
    void mouseRelease();
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
private:
    int fRectIndent = 3;
    QPen fPen = QPen(Qt::black, 1, Qt::SolidLine);
    QBrush fBrush = QBrush(Qt::red);
};

#endif // ZGRAPH_H

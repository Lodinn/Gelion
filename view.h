#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsView>
#include <QPixmap>

class gQGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit gQGraphicsView(QGraphicsScene *scene = 0);
public slots:
    
protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool _pan;
    int _panX0, _panY0;
signals:
  void point_picked(QPointF);

};

#endif // VIEW_H

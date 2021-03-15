#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>

class ImageLabel: public QLabel
{

    Q_OBJECT


public:
    ImageLabel(QWidget *parent = 0);

    void setNeedDrawLine(bool flag) {
        needDrawLine = flag;
    }
    void setDeletePointAndLine(bool flag) {
        deletePointAndLine = flag;
    }

    bool pointInRect (const QPoint&, const QRect&);
    void initPoints(QString line, int scale);
    void clearPointsAndLines();
    bool writeMinutiaeToFile(QString& filename, int scale);
    void drawInitialPointsLines();

private:
    double calOrientation (QList<QPoint>& line) const;
    void drawPointsLines(QPainter* painter);
    void drawPoints(QPainter* painter);
    void drawLines(QPainter* painter);

private slots:
    void mousePressEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent * e);

private:
    QLabel* mainLabel;

    int firstX;
    int firstY;
    int secondX;
    int secondY;

    QList<QList<QPoint>> lines;
    QList<QPoint> points;

    bool firstClick;
    bool paintFlag;
    bool paintRectFlag;
    bool needDrawLine = false;
    bool deletePointAndLine = false;

    bool initialDrawing = false;
};


#endif // IMAGELABEL_H


#include "imagelabel.h"

#include <QtGui>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QtMath>


ImageLabel::ImageLabel(QWidget *parent)
    : QLabel(parent) {
    mainLabel = new QLabel;
    mainLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mainLabel->adjustSize();
    mainLabel->setScaledContents(true);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(mainLabel);

    setLayout(hLayout);
}

void ImageLabel :: mousePressEvent(QMouseEvent *e) {
        firstX=0;
        firstY=0;
        firstClick=true;
        paintFlag=false;

        if(e->button() == Qt::LeftButton && firstClick) {
            firstX = e->x();
            firstY = e->y();
            firstClick = false;
            paintFlag = true;
            paintRectFlag = true;
        }
}

void ImageLabel::mouseMoveEvent(QMouseEvent *event) {
    secondX = event->x();
    secondY = event->y();

    update();
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *event) {
    secondX = event->x();
    secondY = event->y();

    paintRectFlag = false;

    update();
}

void ImageLabel :: paintEvent(QPaintEvent * e) {
    QLabel::paintEvent(e);
    if (initialDrawing) {
        QPainter* painter = new QPainter(this);
        drawPointsLines(painter);

        initialDrawing = false;
        delete painter;
    }

    if(paintFlag) {
        QPainter* painter = new QPainter(this);

        if (!deletePointAndLine) {
            if (!needDrawLine) {    // draw a point
                QPoint p1(firstX, firstY);
                if (points.size() == lines.size()) {
                    points.append(p1);
                }
                else {
                    while (points.size() > lines.size()) points.pop_back();
                }
                drawPointsLines(painter);
            }
            else {                 // draw a line
                drawPoints(painter);

                QPoint p2(firstX, firstY);

                QList<QPoint> line;
                line.append(points[points.size() - 1]);
                line.append(p2);

                lines.append(line);

                drawLines(painter);
            }
        }

        else {
            if (paintRectFlag) {
                QRect rect(firstX, firstY, secondX - firstX, secondY - firstY);
                QPen paintPenRect(Qt::green);
                paintPenRect.setWidth(3);
                painter->setPen(paintPenRect);
                painter->drawRect(rect);

                int index = 0;
                for (; index < points.size(); index++) {
                    if (pointInRect(points[index], rect)) {
                        break;
                    }
                }
                points.removeAt(index);
                lines.removeAt(index);
            }

            drawPointsLines(painter);
        }

        delete painter;
    }
}

void ImageLabel::drawInitialPointsLines() {
    initialDrawing = true;
}

bool ImageLabel::pointInRect(const QPoint& p, const QRect& rect)
{
    int px = p.x();
    int py = p.y();

    int rx = rect.x();
    int ry = rect.y();
    int w = rect.width();
    int h = rect.height();

    return rx <= px && px <= rx + w && ry <= py && py <= ry + h;
}

void ImageLabel::initPoints(QString line, int scale)
{
    QStringList pieces = line.split(" ");
    int w = pieces[0].toInt();
    int h = pieces[1].toInt();
    double orientation = pieces[2].toDouble();
    int w2 = w + static_cast<int>(qCos(orientation) * 20);
    int h2 = h - static_cast<int>(qSin(orientation) * 20);

    QPoint point(w * scale, h * scale);
    points.append(point);

    QPoint point2(w2 * scale, h2 * scale);
    QList<QPoint> line2;
    line2.append(point);
    line2.append(point2);
    lines.append(line2);
}

void ImageLabel::clearPointsAndLines()
{
    points.clear();
    lines.clear();
}

bool ImageLabel::writeMinutiaeToFile(QString &filename, int scale)
{
    QFile savedFile(filename);
    savedFile.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream savedFileStram(&savedFile);

    for (int i = 0; i < lines.size(); i++) {
        QPoint last_point;
        if (i > 0) {
            last_point = lines[i-1][0];
        }

        QList<QPoint> line = lines[i];

        QPoint point1 = line[0];
        double orientation = calOrientation(line);

        if (point1.x() != last_point.x() || point1.y() != last_point.y()) {
            savedFileStram << point1.x() / scale << " " << point1.y() / scale <<  " " << orientation << "\n";
        }
    }

    savedFile.close();
    return true;
}

double ImageLabel::calOrientation(QList<QPoint>& line) const
{
    QPoint point1 = line[0];
    QPoint point2 = line[1];

    int x = point2.x() - point1.x();
    int y = point2.y() - point1.y();

    double rawOrientation = qAtan2(y, x) > 0 ? qAtan2(y, x) : 3.1415926 * 2 + qAtan2(y, x);

    double orientation = 3.1415926 * 2 - rawOrientation;

    return orientation;
}

void ImageLabel::drawPointsLines(QPainter* painter) {
    QPen paintPen(Qt::red);
    paintPen.setWidth(5);
    painter->setPen(paintPen);

    foreach (const QPoint& point, points) {
        painter->drawPoint(point);
    }

    paintPen.setWidth(2);
    painter->setPen(paintPen);

    foreach (const QList<QPoint>& line, lines) {
        painter->drawLine(line[0], line[1]);
    }
}

void ImageLabel::drawPoints(QPainter *painter)
{
    QPen paintPen(Qt::red);
    paintPen.setWidth(5);
    painter->setPen(paintPen);

    foreach (const QPoint& point, points) {
        painter->drawPoint(point);
    }
}

void ImageLabel::drawLines(QPainter *painter)
{
    QPen paintPen(Qt::red);
    paintPen.setWidth(2);
    painter->setPen(paintPen);

    foreach (const QList<QPoint>& line, lines) {
        painter->drawLine(line[0], line[1]);
    }
}


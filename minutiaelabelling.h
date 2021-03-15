#ifndef MINUTIAELABELLING_H
#define MINUTIAELABELLING_H


#include "imagelabel.h"
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QImage>
#if defined(QT_PRINTSUPPORT_LIB)
#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printer)
#    include <QPrinter>
#  endif
#endif

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE

class MinutiaeLabelling : public QMainWindow
{
    Q_OBJECT

public:
    MinutiaeLabelling(QWidget *parent = nullptr);
    ~MinutiaeLabelling();

    bool loadFile(const QString &);

private slots:
    void open();
    void saveAs();
    void print();
    void copy();
    void paste();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void about();

private:
    void createActions();
    void createMenus();
    void updateActions();
    bool saveFile(const QString &fileName);
    void setImage(const QImage &newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    QImage image;
    ImageLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor = 1;

    int imageDisplayScale = 2;


private slots:
    void beginDrawLine();
    void beginDrawPoint();
    void beginDelete();

    void openOriginDir();
    void openSavedDir();

    void saveTxtFile();

private:
    QPushButton* btnDrawLine;
    QPushButton* btnDrawPoint;
    QPushButton* btnDelete;

    QPushButton* btnOpenOriginTxtDir;
    QPushButton* btnOpenSavedTxtDir;

    QString imageName;
    QString originTxtDir;
    QString savedTxtDir;
    QString defaultOpenDir;

    QLabel* lblOriginTxtDir;
    QLabel* lblSavedTxtDir;

    QPushButton* btnSaveTxtFile;

private:
    bool readOriginTxt();


#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    QPrinter printer;
#endif

    QAction *saveAsAct;
    QAction *printAct;
    QAction *copyAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
};

#endif // MINUTIAELABELLING_H

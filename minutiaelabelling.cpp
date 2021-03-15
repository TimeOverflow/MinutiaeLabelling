#include "minutiaelabelling.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QStandardPaths>
#include <QStatusBar>

#include <QDebug>
#include <QMouseEvent>

#if defined(QT_PRINTSUPPORT_LIB)
#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printdialog)
#    include <QPrintDialog>
#  endif
#endif

MinutiaeLabelling::~MinutiaeLabelling()
{
}

MinutiaeLabelling::MinutiaeLabelling(QWidget *parent)
    : QMainWindow(parent), imageLabel(new ImageLabel)
    , scrollArea(new QScrollArea), defaultOpenDir("E:\\Fingerprint\\Database\\plot_label\\unsw_downsample_2000")
{
    imageLabel->setBackgroundRole(QPalette::Light);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Light);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    setCentralWidget(scrollArea);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize());

    QFont font("Arial", 12);

    // origin txt dir:
    QLabel* lbl1 = new QLabel(this);
    lbl1->resize(100, 30);
    lbl1->setText("Origin txt dir:");
    lbl1->setFont(font);
    lbl1->move(1100, 600);

    // show original txt dir
    lblOriginTxtDir = new QLabel(this);
    lblOriginTxtDir->resize(300, 30);
    lblOriginTxtDir->setFont(font);
    lblOriginTxtDir->move(1200, 600);
    lblOriginTxtDir->setStyleSheet("QLabel { background-color : white; color : black; }");

    // save txt dir
    QLabel* lbl2 = new QLabel(this);
    lbl2->resize(100, 30);
    lbl2->setText("Saved txt dir:");
    lbl2->setFont(font);
    lbl2->move(1100, 650);

    // show saved txt dir
    lblSavedTxtDir = new QLabel(this);
    lblSavedTxtDir->resize(300, 30);
    lblSavedTxtDir->setFont(font);
    lblSavedTxtDir->move(1200, 650);
    lblSavedTxtDir->setStyleSheet("QLabel { background-color : white; color : black; }");

    btnDrawLine = new QPushButton(this);
    btnDrawLine->setText("draw line");
    btnDrawLine->setFont(font);
    btnDrawLine->resize(100, 30);
    btnDrawLine->move(1100, 400);

    btnDrawPoint = new QPushButton(this);
    btnDrawPoint->setText("draw point");
    btnDrawPoint->setFont(font);
    btnDrawPoint->resize(100, 30);
    btnDrawPoint->move(1200, 400);

    btnDelete = new QPushButton(this);
    btnDelete->setText("delete point and line");
    btnDelete->setFont(font);
    btnDelete->resize(200, 30);
    btnDelete->move(1100, 450);

    btnOpenOriginTxtDir = new QPushButton(this);
    btnOpenOriginTxtDir->setText("open origin dir");
    btnOpenOriginTxtDir->setFont(font);
    btnOpenOriginTxtDir->resize(200, 30);
    btnOpenOriginTxtDir->move(1100, 500);

    btnOpenSavedTxtDir = new QPushButton(this);
    btnOpenSavedTxtDir->setText("open saved dir");
    btnOpenSavedTxtDir->setFont(font);
    btnOpenSavedTxtDir->resize(200, 30);
    btnOpenSavedTxtDir->move(1100, 550);

    btnSaveTxtFile = new QPushButton(this);
    btnSaveTxtFile->setText("save noted minutiae");
    btnSaveTxtFile->setFont(font);
    btnSaveTxtFile->resize(200, 30);
    btnSaveTxtFile->move(1100, 300);

    connect(btnDrawLine, SIGNAL(clicked()), this, SLOT(beginDrawLine()));
    connect(btnDrawPoint, SIGNAL(clicked()), this, SLOT(beginDrawPoint()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(beginDelete()));
    connect(btnOpenOriginTxtDir, SIGNAL(clicked()), this, SLOT(openOriginDir()));
    connect(btnOpenSavedTxtDir, SIGNAL(clicked()), this, SLOT(openSavedDir()));
    connect(btnSaveTxtFile, SIGNAL(clicked()), this, SLOT(saveTxtFile()));
}

bool MinutiaeLabelling::loadFile(const QString &fileName)
{
    QString imageFilePath = fileName;

    QStringList pieces = imageFilePath.split( "/" );
    imageName = pieces.value( pieces.length() - 1 );


    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }


    setImage(newImage);

    setWindowFilePath(fileName);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
    statusBar()->showMessage(message);
    return true;
}

void MinutiaeLabelling::setImage(const QImage &newImage)
{
    imageLabel->clearPointsAndLines();
    image = newImage;

    QPixmap pm = QPixmap::fromImage(image);
    imageLabel->setPixmap(pm.scaled(image.size() * imageDisplayScale, Qt::KeepAspectRatio));

    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();

    readOriginTxt();
    imageLabel->drawInitialPointsLines();
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode, const QString & defaultDir)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
//        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
//        dialog.setDirectory(picturesLocations.isEmpty() ? defaultDir : picturesLocations.last());
        dialog.setDirectory(defaultDir);
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/bmp");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("bmp");
}

void MinutiaeLabelling::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen, defaultOpenDir);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

// defined by zhangzhao
void MinutiaeLabelling::beginDrawLine()
{
    imageLabel->setNeedDrawLine(true);
    imageLabel->setDeletePointAndLine(false);
}

void MinutiaeLabelling::beginDrawPoint()
{
    imageLabel->setNeedDrawLine(false);
    imageLabel->setDeletePointAndLine(false);
}

void MinutiaeLabelling::beginDelete()
{
    imageLabel->setDeletePointAndLine(true);
}

void MinutiaeLabelling::openOriginDir()
{
    QString path = defaultOpenDir;
    QString title = "choose directory";
    originTxtDir = QFileDialog::getExistingDirectory(this, title, path, QFileDialog::ShowDirsOnly);

    lblOriginTxtDir->setText(originTxtDir);
}

void MinutiaeLabelling::openSavedDir()
{
    QString path = defaultOpenDir;
    QString title = "choose directory";
    savedTxtDir = QFileDialog::getExistingDirectory(this, title, path, QFileDialog::ShowDirsOnly);

    lblSavedTxtDir->setText(savedTxtDir);
}

void MinutiaeLabelling::saveTxtFile()
{
    QStringList imageNamePieces = imageName.split(".");
    QString imageNamePrefix = imageNamePieces[0];
    QString savedTxtPath = savedTxtDir + "/" + imageNamePrefix + ".txt";

    imageLabel->writeMinutiaeToFile(savedTxtPath, imageDisplayScale);
}

bool MinutiaeLabelling::readOriginTxt()
{
    QStringList imageNamePieces = imageName.split(".");
    QString imageNamePrefix = imageNamePieces[0];
    QString originTxtPath = originTxtDir + "/" + imageNamePrefix + ".txt";

    QFile originTxt(originTxtPath);
    originTxt.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream inStream(&originTxt);

    while (!inStream.atEnd()) {
        imageLabel->initPoints(inStream.readLine(), imageDisplayScale);
    }

    originTxt.close();

    return true;
}

// functions from ImageViewer example of Qt
bool MinutiaeLabelling::saveFile(const QString &fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(image)) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}

void MinutiaeLabelling::saveAs()
{
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave, defaultOpenDir);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}


void MinutiaeLabelling::print()
{
//    Q_ASSERT(!imageLabel->pixmap(Qt::ReturnByValue).isNull());
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printdialog)
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        const QPixmap* pixmap = imageLabel->pixmap();
        QRect rect = painter.viewport();
        QSize size = pixmap->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(pixmap->rect());
        painter.drawPixmap(0, 0, *pixmap);
    }
#endif
}

void MinutiaeLabelling::copy()
{
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(image);
#endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD

void MinutiaeLabelling::paste()
{
#ifndef QT_NO_CLIPBOARD
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        statusBar()->showMessage(tr("No image in clipboard"));
    } else {
        setImage(newImage);
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
            .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}

void MinutiaeLabelling::zoomIn()
{
    scaleImage(1.25);
}

void MinutiaeLabelling::zoomOut()
{
    scaleImage(0.8);
}

void MinutiaeLabelling::normalSize()

{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void MinutiaeLabelling::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        normalSize();
    updateActions();
}

void MinutiaeLabelling::about()
{
    QMessageBox::about(this, tr("About Image Viewer"),
            tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
               "and QScrollArea to display an image. QLabel is typically used "
               "for displaying a text, but it can also display an image. "
               "QScrollArea provides a scrolling view around another widget. "
               "If the child widget exceeds the size of the frame, QScrollArea "
               "automatically provides scroll bars. </p><p>The example "
               "demonstrates how QLabel's ability to scale its contents "
               "(QLabel::scaledContents), and QScrollArea's ability to "
               "automatically resize its contents "
               "(QScrollArea::widgetResizable), can be used to implement "
               "zooming and scaling features. </p><p>In addition the example "
               "shows how to use QPainter to print an image.</p>"));
}

void MinutiaeLabelling::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &MinutiaeLabelling::open);
    openAct->setShortcut(QKeySequence::Open);

    saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &MinutiaeLabelling::saveAs);
    saveAsAct->setEnabled(false);

    printAct = fileMenu->addAction(tr("&Print..."), this, &MinutiaeLabelling::print);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setEnabled(false);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &MinutiaeLabelling::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    QAction *pasteAct = editMenu->addAction(tr("&Paste"), this, &MinutiaeLabelling::paste);
    pasteAct->setShortcut(QKeySequence::Paste);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &MinutiaeLabelling::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &MinutiaeLabelling::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &MinutiaeLabelling::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &MinutiaeLabelling::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &MinutiaeLabelling::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}


void MinutiaeLabelling::updateActions()
{
    saveAsAct->setEnabled(!image.isNull());
    copyAct->setEnabled(!image.isNull());
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void MinutiaeLabelling::scaleImage(double factor)
{
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void MinutiaeLabelling::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}












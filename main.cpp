#include "minutiaelabelling.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MinutiaeLabelling w;
    w.show();
    return a.exec();
}

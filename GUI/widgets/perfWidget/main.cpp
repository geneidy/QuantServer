#include <qglobal.h>
#if (QT_VERSION < QT_VERSION_CHECK(5 ,0 ,0))
#include <QtGui/QApplication>
#else
#include <QtWidgets/QApplication>
#endif
#include "perfwidget.h"
#include <iostream>
#include <cstdio>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PerfWidget w;
    w.show();

    return a.exec();
}

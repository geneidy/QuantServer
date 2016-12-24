#######
#
#######

#include($$system(pwd)/widgets/perfWidget/perfWidget.pro)

TEMPLATE = app
TARGET = bin/qtxgui
DEPENDPATH += . widgets/perfWidget \
                widgets/digitalclock \
                #widgets/led-designer-plugin \
                widgets/resourcesWidget \
                ../Include
INCLUDEPATH += . widgets/perfWidget \
                 widgets/digitalclock \
                 #widgets/led-designer-plugin \
                 widgets/resourcesWidget \
                 ../Include
RESOURCES += resources.qrc
# Input
HEADERS +=  QtxGui.h \
            dialogs/connDialog.h \
            dialogs/configdialog.h \
            dialogs/configpages.h \
            ../Include/Settings.h \
            ../Include/Includes.h \
            ../Include/NQTV.h \
            perfwidget.h \
            dataQuery/cpudataquery.h \
            dataQuery/dataquery.h \
            dataQuery/queryresource.h \
            dataQuery/ramdataquery.h \
            painters/barspainter.h \
            painters/graphmulticolorpainter.h \
            painters/graphpainter.h \
            painters/painterutils.h \
            painters/performancepainter.h \
            widgets/cpuwidget.h \
            widgets/performancewidget.h \
            widgets/ramwidget.h \
            digitalclock.h \            
            #LED.h \
            memoryconversion.h \
            resourcesworker.h \
            workerthread.h \
            resourcesWidget.h
FORMS +=    QtxGui.ui \
            dialogs/connDialog.ui \
            perfwidget.ui \
            resourcesWidget.ui
SOURCES +=  main.cpp \
            QtxGui.cpp \
            dialogs/configdialog.cpp \
            dialogs/configpages.cpp \
            dialogs/connDialog.cpp \
            perfwidget.cpp \
            dataQuery/cpudataquery.cpp \
            dataQuery/dataquery.cpp \
            dataQuery/queryresource.cpp \
            dataQuery/ramdataquery.cpp \
            painters/barspainter.cpp \
            painters/graphmulticolorpainter.cpp \
            painters/graphpainter.cpp \
            painters/painterutils.cpp \
            painters/performancepainter.cpp \
            widgets/cpuwidget.cpp \
            widgets/performancewidget.cpp \
            widgets/ramwidget.cpp \
            digitalclock.cpp \
            #LED.cpp \
            memoryconversion.cpp \
            resourcesworker.cpp \
            workerthread.cpp \
            resourcesWidget.cpp

LIBS += -L"libprocps" -lprocps            
#include(widgets/perfWidget/perfWidget.pro)


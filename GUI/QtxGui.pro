#######
#
#######

TEMPLATE = app
TARGET = bin/qtxgui
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += QtxGui.h dialogs/connectionDialog.h dialogs/configdialog.h dialogs/configpages.h ../Include/Settings.h ../Include/Includes.h
FORMS += QtxGui.ui dialogs/connectionDialog.ui
SOURCES += main.cpp QtxGui.cpp dialogs/configdialog.cpp dialogs/configpages.cpp dialogs/connectionDialog.cpp

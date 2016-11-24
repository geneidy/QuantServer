#######
#
#######

TEMPLATE = app
TARGET = QuantServer
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += QuantServerMainWindow.h connectionDialog.h ../Include/Settings.h ../Include/Includes.h
FORMS += QuantServerMainWindow.ui connectionDialog.ui
SOURCES += main.cpp QuantServerMainWindow.cpp connectionDialog.cpp

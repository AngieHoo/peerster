######################################################################
# Automatically generated by qmake (3.0) Fri Sep 15 12:26:38 2017
######################################################################
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = peerster
INCLUDEPATH += .
DEPENDPATH += .

QMAKE_CXXFLAGS += -std=c++0x

QT += network core gui


HEADERS += chatdialog.h netsocket.h peer.h textinput.h \
    control.h \
    model.h \
    lib.h
SOURCES += chatdialog.cpp main.cc netsocket.cpp peer.cpp textinput.cpp \
    control.cpp \
    model.cpp

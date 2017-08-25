#-------------------------------------------------
#
# Project created by QtCreator 2016-05-25T11:37:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GQMetadataXML
TEMPLATE = app

RC_ICONS = Xml_tool.ico

SOURCES += main.cpp\
        mainwindow.cpp \
    xmlstreamreader.cpp \
    myogr.cpp \
    setdialog.cpp \
    core.cpp

HEADERS  += mainwindow.h \
    xmlstreamreader.h \
    myogr.h \
    setdialog.h \
    core.h

FORMS    += mainwindow.ui \
    setdialog.ui

win32: LIBS += -L C:/gdal201/lib/ -lgdal

INCLUDEPATH += C:/gdal201/include
DEPENDPATH += C:/gdal201/include

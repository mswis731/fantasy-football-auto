#-------------------------------------------------
#
# Project created by QtCreator 2016-12-18T20:43:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fantasy-football-auto
TEMPLATE = app


SOURCES += main.cpp\
           logindialog.cpp \
           mainwindow.cpp \
           secret.cpp \
           jsoncpp/jsoncpp.cpp

HEADERS  += logindialog.h \
            mainwindow.h \
            secret.h \
            jsoncpp/json/json-forwards.h \
            jsoncpp/json/json.h

FORMS    += logindialog.ui \
            mainwindow.ui

RESOURCES += \
    Resources/resources.qrc

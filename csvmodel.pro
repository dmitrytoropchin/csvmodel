BUILD_DIR   = .build
MOC_DIR     = $$BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR
RCC_DIR     = $$BUILD_DIR
UI_DIR      = $$BUILD_DIR

QT          = core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE    = app
TARGET      = csvmodel
CODECFORSRC = UTF-8

INCLUDEPATH += \
    source

HEADERS = \
    source/csvmodel.h

SOURCES = \
    source/main.cpp \
    source/csvmodel.cpp

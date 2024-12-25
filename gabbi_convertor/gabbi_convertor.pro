QT -= gui
QT += widgets
QT += concurrent

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/../unicon_core

INCLUDEPATH += $$PWD/../dde_lib

SOURCES += \
        ../unicon_core/oscdatalogger.cpp \
        ../unicon_core/oscdatalogger_v1_2.cpp \
        main.cpp

HEADERS += \
        ../unicon_core/dde_lib/DDE_TYPES.h \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


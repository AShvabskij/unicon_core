CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    db_sqlib.cpp \
    shell.c \
    sqlite3.c

unix: SOURCES += \
    sqlite3_lib.cpp

HEADERS += \
    db_sqlib.h \
    sqlite3_lib.h

INCLUDEPATH += $$PWD/../../

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

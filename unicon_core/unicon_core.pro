QT -= gui
QT += core network websockets

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        core.cpp \
        basereqhandler.cpp \
        devicehandler.cpp \
        oschandler.cpp \
        paramshandler.cpp \
        requestmanager.cpp \
        responsemanager.cpp \
        socketserver.cpp \
        streammanager.cpp


HEADERS += \
    core.h \
    basereqhandler.h \
    devicehandler.h \
    ireqhandler.h \
    oschandler.h \
    paramshandler.h \
    requestmanager.h \
    responsemanager.h \
    socketserver.h \
    streammanager.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../dde_lib/release/ -lDDE_lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../dde_lib/debug/ -lDDE_lib
else:unix: LIBS += -L$$PWD/../dde_lib/debug/ -lDDE_lib

# INCLUDEPATH += $$PWD/../utils/IPCmemLib
INCLUDEPATH += $$PWD/../dde_lib

# DEPENDPATH += $$PWD/../utils/SQLite3Lib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../dde_lib/release/libDDE_lib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../dde_lib/debug/libDDE_lib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../dde_lib/release/DDE_lib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../dde_lib/debug/DDE_lib.lib
else:unix: PRE_TARGETDEPS += $$PWD/../dde_lib/debug/libDDE_lib.a

SUBDIRS += \
    ../dde_lib/DDE_lib.pro

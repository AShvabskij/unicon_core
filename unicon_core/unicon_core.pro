QT -= gui
QT += core network websockets
QT += concurrent

CONFIG += c++11 console
CONFIG -= app_bundle
LIBS += -lpthread
unix: LIBS += -lrt
unix: LIBS += -ldl
unix: LIBS += -lsqlite3

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        dde_dispatcher.cpp \
        main.cpp \
        core.cpp \
        basereqhandler.cpp \
        devicehandler.cpp \
        oscdataservice.cpp \
        oscdatastorage.cpp \
        oscdatastorage_v1_2.cpp \
        oschandler.cpp \
        oscstateservice.cpp \
        paramshandler.cpp \
        requestmanager.cpp \
        responsemanager.cpp \
        socketserver.cpp \
        streammanager.cpp \
        systemservice.cpp


HEADERS += \
    core.h \
    basereqhandler.h \
    dde_dispatcher.h \
    device_types.h \
    devicehandler.h \
    ireqhandler.h \
    osc_types.h \
    oscdataservice.h \
    oscdatastorage.h \
    oscdatastorage_v1_2.h \
    oschandler.h \
    oscstateservice.h \
    paramshandler.h \
    requestmanager.h \
    responsemanager.h \
    socketserver.h \
    streammanager.h \
    systemservice.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../dde_lib/release/ -lDDE_lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../dde_lib/debug/ -lDDE_lib
else:unix: LIBS += -L$$PWD/../dde_lib/ -lDDE_lib

INCLUDEPATH += $$PWD/../dde_lib
DEPENDPATH += $$PWD/../dde_lib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../dde_lib/release/libDDE_lib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../dde_lib/debug/libDDE_lib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../dde_lib/release/DDE_lib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../dde_lib/debug/DDE_lib.lib
else: PRE_TARGETDEPS += $$PWD/../dde_lib/libDDE_lib.a

SUBDIRS += \
    ../dde_lib/DDE_lib.pro

unix:!macx: LIBS += -L$$OUT_PWD/../dde_lib/utils/IPCmemLib/ -lipcmem_lib
unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../dde_lib/utils/IPCmemLib/libipcmem_lib.a


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dde_lib/utils/sql3_db/release/ -lsql3_db
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dde_lib/utils/sql3_db/debug/ -lsql3_db
else:unix:!macx: LIBS += -L$$OUT_PWD/../dde_lib/utils/sql3_db/ -lsql3_db

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../dde_lib/utils/sql3_db/release/libsql3_db.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../dde_lib/utils/sql3_db/debug/libsql3_db.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../dde_lib/utils/sql3_db/release/sql3_db.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../dde_lib/utils/sql3_db/debug/sql3_db.lib
else:unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../dde_lib/utils/sql3_db/libsql3_db.a

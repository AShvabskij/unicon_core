CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DDE_OSC_DISPATCHER.cpp \
    DDE_TOP.cpp \
    DDE_EVLOG.cpp \
    DDE_OSC.cpp \
    DDE_OSC_EMUL.cpp \
    DDE_OSC_FILE.cpp \
    DDE_PARAMS_EMUL.cpp \
    DDE_PARAMS_FILE.cpp \
    DDE_EMUL.cpp \
    csvfile.cpp \
    services/oscfileservice.cpp \
    services/oscipcservice.cpp \
    services/oscpagebinservice.cpp \
    services/oscpagetxtservice.cpp

unix: SOURCES += \
    DDE_PARAMS.cpp

HEADERS += \
    DDE_DEVICES_TYPE.h \
    DDE_INTERFACES.h \
    DDE_OSC_DISPATCHER.h \
    cpp_inc.h \
    DDE_TOP.h \
    DDE_EVLOG.h \
    DDE_EVLOG_TYPES.h \
    DDE_OSC.h \
    DDE_OSC_EMUL.h \
    DDE_OSC_FILE.h \
    DDE_OSC_TYPES.h \
    DDE_PARAMS.h \
    DDE_PARAMS_EMUL.h \
    DDE_PARAMS_FILE.h \
    DDE_PARAMS_TYPE.h \
    DDE_TYPES.h \
    DDE_EMUL.h \
    services/oscfileservice.h \
    services/oscipcservice.h \
    services/oscpagebinservice.h \
    services/oscpagetxtservice.h

INCLUDEPATH += $$PWD/utils/csvfile
INCLUDEPATH += $$PWD/utils/IPCmemLib
DEPENDPATH += $$PWD/utils/IPCmemLib

INCLUDEPATH += $$PWD/services

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/utils/IPCmemLib/bin/x64/release/ -lipcmem_lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/utils/IPCmemLib/bin/x64/debug/ -lipcmem_lib
unix:!macx: LIBS += -L$$OUT_PWD/utils/IPCmemLib/ -lipcmem_lib

unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/utils/IPCmemLib/libipcmem_lib.a

# unix:!macx: LIBS += -L$$OUT_PWD/./ -lDDE_lib

# unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/./libDDE_lib.a



INCLUDEPATH += $$PWD/utils/sql3_db
DEPENDPATH += $$PWD/utils/sql3_db

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/utils/sql3_db/release/ -lsql3_db
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/utils/sql3_db/debug/ -lsql3_db
else:unix:!macx: LIBS += -L$$OUT_PWD/utils/sql3_db/ -lsql3_db

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/utils/sql3_db/release/libsql3_db.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/utils/sql3_db/debug/libsql3_db.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/utils/sql3_db/release/sql3_db.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/utils/sql3_db/debug/sql3_db.lib
else:unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/utils/sql3_db/libsql3_db.a

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

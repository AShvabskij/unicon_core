CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DDE.cpp \
    DDE_EVLOG.cpp \
    DDE_OSC.cpp \
    DDE_OSC_EMUL.cpp \
    DDE_OSC_FILE.cpp \
    DDE_PARAMS.cpp \
    DDE_PARAMS_EMUL.cpp \
    DDE_PARAMS_FILE.cpp \
    DDE_EMUL.cpp \
    csvfile.cpp

HEADERS += \
    DDE_INTERFACES.h \
    cpp_inc.h \
    DDE.h \
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
    DDE_EMUL.h

INCLUDEPATH += $$PWD/utils/IPCmemLib
INCLUDEPATH += $$PWD/utils/sql3_db
INCLUDEPATH += $$PWD/utils/csvfile

DEPENDPATH += $$PWD/utils/IPCmemLib
DEPENDPATH += $$PWD/utils/sql3_db

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/utils/IPCmemLib/bin/x64/release/ -lipcmem_lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/utils/IPCmemLib/bin/x64/debug/ -lipcmem_lib



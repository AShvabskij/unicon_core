CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DDE/DDE.cpp \
    DDE/DDE_EVLOG.cpp \
    DDE/DDE_OSC.cpp \
    DDE/DDE_PARAMS.cpp \
    DDE/DDE_PARAMS_EMUL.cpp \
    DDE_EMUL.cpp \
    dde_lib.cpp \
    DDE_CAN.cpp

HEADERS += \
    DDE/DDE.h \
    DDE/DDE_EVLOG.h \
    DDE/DDE_OSC.h \
    DDE/DDE_PARAMS.h \
    DDE/DDE_PARAMS_EMUL.h \
    DDE/DDE_PARAMS_TYPES.h \
    DDE/DDE_types.h \
    DDE/my_func.h \
    DDE_EMUL.h \
    dde_lib.h \
    DDE_CAN.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

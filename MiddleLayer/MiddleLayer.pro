QT += core network websockets qml quick

CONFIG += c++11 qml_debug qmltypes

QML_IMPORT_NAME = solcon.qmlmodels
QML_IMPORT_MAJOR_VERSION = 1

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        core.cpp \
        basereqhandler.cpp \
        devicehandler.cpp \
        mainwindowvm.cpp \
        paramshandler.cpp \
        requestmanager.cpp \
        responsemanager.cpp \
        socketserver.cpp \
        main.cpp \
        streammanager.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    core.h \
    basereqhandler.h \
    devicehandler.h \
    ireqhandler.h \
    mainwindowvm.h \
    paramshandler.h \
    requestmanager.h \
    responsemanager.h \
    socketserver.h \
    streammanager.h

OTHER_FILES = \
    $$files(*.qml) \
    components

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../DDE_CAN_LIB/release/ -lDDE_CAN_Lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../DDE_CAN_LIB/debug/ -lDDE_CAN_Lib
else:unix: LIBS += -L$$PWD/../DDE_CAN_LIB/ -lDDE_CAN_Lib

INCLUDEPATH += $$PWD/../DDE_CAN_LIB
DEPENDPATH += $$PWD/../DDE_CAN_LIB

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../DDE_CAN_LIB/release/libDDE_CAN_Lib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../DDE_CAN_LIB/debug/libDDE_CAN_Lib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../DDE_CAN_LIB/release/DDE_CAN_Lib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../DDE_CAN_LIB/debug/DDE_CAN_Lib.lib
else:unix: PRE_TARGETDEPS += $$PWD/../DDE_CAN_LIB/libDDE_CAN_Lib.a

SUBDIRS += \
    ../DDE_CAN_LIB/DDE_CAN_Lib.pro

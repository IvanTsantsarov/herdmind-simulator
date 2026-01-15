QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

DEFINES += SIMULATION

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    animal.cpp \
    focusanim.cpp \
    hardware/bolus/accel.cpp \
    hardware/bolus/bolus.cpp \
    hardware/netnode.cpp \
    hardware/collar/collar.cpp \
    hardware/gateway/gateway.cpp \
    hardware/protocol.cpp \
    hardware/tools.cpp \
    herd.cpp \
    main.cpp \
    mainwindow.cpp \
    meadow.cpp \
    network.cpp \
    scene.cpp \
    sceneview.cpp \
    shepherd.cpp \
    simtools.cpp

HEADERS += \
    animal.h \
    focusanim.h \
    hardware/bolus/accel.h \
    hardware/bolus/bolus.h \
    hardware/bolus/includes.h \
    hardware/netnode.h \
    hardware/collar/collar.h \
    defines.h \
    hardware/gateway/gateway.h \
    hardware/protocol.h \
    hardware/tools.h \
    herd.h \
    mainwindow.h \
    meadow.h \
    network.h \
    scene.h \
    sceneview.h \
    shepherd.h \
    simtools.h

FORMS += \
    focusanim.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    trash.txt

RESOURCES += \
    res/res.qrc

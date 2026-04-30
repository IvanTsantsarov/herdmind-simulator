QT       += core gui widgets positioning network mqtt

CONFIG += c++17

DEFINES += SIMULATION

# Link OpenSSL
LIBS += -lcrypto


# Disable warnings
# QMAKE_CXXFLAGS += -Wclazy-qstring-arg

CONFIG(debug, debug|release) {
# DEBUG
}

CONFIG(release, debug|release) {
# RELEASE
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    animal.cpp \
    animal_names.cpp \
    apimqtt.cpp \
    apirest.cpp \
    apirest_response.cpp \
    devmanager.cpp \
    dialogconsole.cpp \
    dialogdevicemsg.cpp \
    focusanim.cpp \
    grpc.cpp \
    hardware/bolus/accel.cpp \
    hardware/bolus/bolus.cpp \
    hardware/collar/collar.cpp \
    hardware/gateway/gateway.cpp \
    hardware/loradev.cpp \
    hardware/tools.cpp \
    herd.cpp \
    main.cpp \
    mainwindow.cpp \
    meadow.cpp \
    mqtt.cpp \
    network.cpp \
    scene.cpp \
    sceneview.cpp \
    shepherd.cpp \
    simtimer.cpp \
    simtools.cpp

HEADERS += \
    animal.h \
    apimqtt.h \
    apirest.h \
    defines_settings.h \
    devmanager.h \
    dialogconsole.h \
    dialogdevicemsg.h \
    focusanim.h \
    grpc.h \
    hardware/bolus/accel.h \
    hardware/bolus/bolus.h \
    hardware/bolus/includes.h \
    hardware/collar/collar.h \
    defines.h \
    hardware/defines.h \
    hardware/gateway/gateway.h \
    hardware/loradev.h \
    hardware/protocol.h \
    hardware/tools.h \
    herd.h \
    mainwindow.h \
    meadow.h \
    mqtt.h \
    network.h \
    scene.h \
    sceneview.h \
    shepherd.h \
    simtimer.h \
    simtools.h

FORMS += \
    dialogconsole.ui \
    dialogdevicemsg.ui \
    focusanim.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    bolus_chirpstack.js \
    collar_chirpstack.js \
    deploy.sh \
    deploy_cmake/CMakeLists.txt \
    res/environment.ini \
    res/settings.ini \
    res/settings_external.ini \
    trash.txt

RESOURCES += \
    res/res.qrc

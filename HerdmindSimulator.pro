QT       += core gui widgets positioning network mqtt

CONFIG += c++17

DEFINES += SIMULATION


# --- Windows Section ---
win32 {
    # Replace with your actual Windows OpenSSL installation path
    OPENSSL_PATH = "C:\Program Files\OpenSSL-Win64"

    INCLUDEPATH += "$$OPENSSL_PATH/include"
    LIBS += -L"$$OPENSSL_PATH/lib/VC/x64/MTd" -lcrypto

    # Use these for older OpenSSL versions if the above fails:
    # LIBS += -L"$$OPENSSL_PATH/lib" -llibeay32 -lssleay32
}

# --- Linux Section ---
unix:!macx {
    # On Linux, OpenSSL is usually in /usr/include and /usr/lib
    # so we often only need to link the libraries.
    LIBS += -lcrypto

    # If using a custom installation path on Linux, uncomment and edit:
    # INCLUDEPATH += "/usr/local/ssl/include"
    # LIBS += -L"/usr/local/ssl/lib"
}


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
    network.cpp \
    scene.cpp \
    sceneview.cpp \
    shepherd.cpp \
    simtools.cpp

HEADERS += \
    animal.h \
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
    network.h \
    scene.h \
    sceneview.h \
    shepherd.h \
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
    res/settings.ini \
    trash.txt

RESOURCES += \
    res/res.qrc

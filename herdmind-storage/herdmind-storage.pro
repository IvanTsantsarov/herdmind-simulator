QT = core mqtt network positioning

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        apirest.cpp \
        main.cpp \
        mqtt.cpp \
        storage.cpp \
        tools.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../hardware/protocol.h \
    apirest.h \
    defines.h \
    mqtt.h \
    storage.h \
    tools.h

RESOURCES += \
    res/res.qrc

QT += core gui quick qml
CONFIG += c++17

TARGET = simple-ui
TEMPLATE = app

SOURCES += \
    main.cpp \
    iotbackend.cpp

HEADERS += \
    iotbackend.h

RESOURCES += \
    qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional include directories
INCLUDEPATH += $$PWD

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

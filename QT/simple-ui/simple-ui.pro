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

DISTFILES += \
    main.qml \
    DashboardCard.qml \
    IoTControlRow.qml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional include directories
INCLUDEPATH += $$PWD

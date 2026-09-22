cmake_minimum_required(VERSION 3.16)

project(simple-ui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt (prefers Qt 6, falls back to Qt 5)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Quick Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Quick Qml)

# Optional QuickControls2
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS QuickControls2)

set(PROJECT_SOURCES
    main.cpp
    iotbackend.h
    iotbackend.cpp
    qml.qrc
)

add_executable(simple-ui ${PROJECT_SOURCES})

target_link_libraries(simple-ui PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Qml
)

if(TARGET Qt${QT_VERSION_MAJOR}::QuickControls2)
    target_link_libraries(simple-ui PRIVATE Qt${QT_VERSION_MAJOR}::QuickControls2)
endif()

# Set include directory
target_include_directories(simple-ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set application output name
set_target_properties(simple-ui PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.iot.simpleui
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
)

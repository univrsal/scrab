cmake_minimum_required(VERSION 3.5)
project(scrab)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    set(scrab_PLATFORM_DEPS
        w32-pthreads
        Qt5::WinMain)
endif()

if(UNIX)
    add_definitions(-DUNIX=1)
endif()

if(APPLE)
    add_definitions(-DMAC_OS=1)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(scrab_SOURCES
    src/screenshot/screenshotgrabber.cpp
    src/screenshot/screenshotgrabber.hpp
    src/screenshot/screengrabberchooserrectitem.cpp
    src/screenshot/screengrabberchooserrectitem.hpp
    src/screenshot/toolboxgraphicsitem.cpp
    src/screenshot/toolboxgraphicsitem.hpp
    src/screenshot/screengrabberoverlayitem.cpp
    src/screenshot/screengrabberoverlayitem.hpp
    src/scrab.cpp)

add_library(scrab MODULE
    ${scrab_SOURCES})

include_directories(
        SYSTEM "${CMAKE_SOURCE_DIR}/UI/obs-frontend-api"
        ${Qt5Core_INCLUDES}
        ${QtWidgets_INCLUDES})

target_link_libraries(scrab
    libobs
    obs-frontend-api
    Qt5::Core
    Qt5::Widgets
    ${scrab_PLATFORM_DEPS})

install_obs_plugin_with_data(scrab data)

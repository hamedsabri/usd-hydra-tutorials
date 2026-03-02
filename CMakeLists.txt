cmake_minimum_required(VERSION 3.17...4.0)

project(HydraTutorials VERSION 0.1.0 LANGUAGES CXX)

# User-configurable path to QT installation directory
set(QT_LOCATION "<your_path_to_qt_install_directory>" CACHE PATH "Qt Install Location")

# User-configurable path to OpenUSD installation directory
set(OPENUSD_LOCATION "<your_path_to_openusd_install_directory>" CACHE PATH "OpenUSD Install Location")

list(APPEND CMAKE_PREFIX_PATH
    ${QT_LOCATION}
    ${OPENUSD_LOCATION}
)

#------------------------------------------------------------------------------
# Packages
#------------------------------------------------------------------------------
# Qt
find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets REQUIRED)
if(Qt6_FOUND)
    message(STATUS "Using Qt ${Qt6_VERSION}")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# OpenUSD
find_package(pxr CONFIG REQUIRED)

if(DEFINED PXR_VERSION)
    math(EXPR USD_MAJOR "${PXR_VERSION} / 10000")
    math(EXPR USD_TEMP  "${PXR_VERSION} % 10000")
    math(EXPR USD_MINOR "${USD_TEMP} / 100")
    math(EXPR USD_PATCH "${USD_TEMP} % 100")

    set(USD_VERSION_STRING "${USD_MAJOR}.${USD_MINOR}.${USD_PATCH}")
    message(STATUS "Using OpenUSD ${USD_VERSION_STRING}")
else()
    message(FATAL_ERROR "OpenUSD not found or PXR_VERSION not defined!")
endif()

#------------------------------------------------------------------------------
# Includes
#------------------------------------------------------------------------------
include(cmake/qtAdvancedDockingSystem.cmake)

#------------------------------------------------------------------------------
# Subdirectories
#------------------------------------------------------------------------------
add_subdirectory(source)
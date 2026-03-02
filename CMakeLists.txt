cmake_minimum_required(VERSION 3.17...4.0)

project(HydraTutorials VERSION 0.1.0 LANGUAGES CXX)

# User-configurable path to QT installation directory
set(QT_LOCATION "<your_path_to_qt_install_directory>" CACHE PATH "Qt Install Location")

list(APPEND CMAKE_PREFIX_PATH
    ${QT_LOCATION}
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

#------------------------------------------------------------------------------
# Includes
#------------------------------------------------------------------------------
include(cmake/qtAdvancedDockingSystem.cmake)

#------------------------------------------------------------------------------
# Subdirectories
#------------------------------------------------------------------------------
add_subdirectory(source)
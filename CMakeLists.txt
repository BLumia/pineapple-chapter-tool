cmake_minimum_required(VERSION 3.5)

project(pineapple-chapter-tool LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (EXE_NAME pchaptertool)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(TagLib REQUIRED taglib)

add_executable(${EXE_NAME}
    main.cpp
#    frameitem.cpp
#    frameitem.h
    chaptertreemanager.cpp
    chaptertreemanager.h
    chaptertreemodel.cpp
    chaptertreemodel.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_include_directories(${EXE_NAME}
PRIVATE
    ${TagLib_INCLUDE_DIRS}
)
target_link_libraries(${EXE_NAME}
PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${TagLib_LINK_LIBRARIES}
)

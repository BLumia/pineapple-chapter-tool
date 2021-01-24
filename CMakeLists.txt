cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

project(pineapple-chapter-tool LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (EXE_NAME pchaptertool)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Mp4v2)

pkg_check_modules(TagLib REQUIRED taglib)

add_executable(${EXE_NAME}
    filehandlerinterface.h
    filehandlerinterface.cpp
    mpegfilehandler.h
    mpegfilehandler.cpp

    main.cpp
    chaptertreemanager.cpp
    chaptertreemanager.h
    chaptertreemodel.cpp
    chaptertreemodel.h
    chaptertreeview.cpp
    chaptertreeview.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

if (NOT Mp4v2_FOUND)
    message (WARNING "Mp4v2 not found!")
    target_compile_definitions(${EXE_NAME} PRIVATE
        NO_LIBMP4V2=1
    )
endif ()

target_include_directories(${EXE_NAME}
PRIVATE
    ${TagLib_INCLUDE_DIRS}
)
target_link_libraries(${EXE_NAME}
PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${TagLib_LINK_LIBRARIES}
)

if (Mp4v2_FOUND)
    target_link_libraries(${EXE_NAME}
    PRIVATE
        Mp4v2::Mp4v2
    )
endif()

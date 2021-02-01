cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

project(pineapple-chapter-tool LANGUAGES CXX)

include (GNUInstallDirs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (EXE_NAME pchaptool)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Mp4v2)

pkg_check_modules(TagLib REQUIRED taglib)

set (PCHAPTOOL_RC_FILES
    # yeah, it's empty.
)

if (WIN32)
    list(APPEND PCHAPTOOL_RC_FILES assets/pineapple-chapter-tool.rc)
endif ()

add_executable(${EXE_NAME}
    filehandlerinterface.h
    filehandlerinterface.cpp
    mpegfilehandler.h
    mpegfilehandler.cpp
    taglibutils_p.h
    taglibutils_p.cpp
    vorbisfilehandler.h
    vorbisfilehandler.cpp
    opusfilehandler.h
    opusfilehandler.cpp
    mp4filehandler.h
    mp4filehandler.cpp
    cuefilehandler.h
    cuefilehandler.cpp

    filehandlermanager.h
    filehandlermanager.cpp

    main.cpp
    chapteritem.cpp
    chapteritem.h
    chaptertreemodel.cpp
    chaptertreemodel.h
    chaptertreeview.cpp
    chaptertreeview.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    ${PCHAPTOOL_RC_FILES}
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

# Extra build settings
if (WIN32)
    set_property (
        TARGET ${EXE_NAME}
        PROPERTY WIN32_EXECUTABLE true
    )
endif ()

# Helper macros for install settings
macro (pchaptool_convert_to_relative_path _var)
    # Make sure _var is a relative path
    if (IS_ABSOLUTE "${${_var}}")
        file (RELATIVE_PATH ${_var} "${CMAKE_INSTALL_PREFIX}" "${${_var}}")
    endif ()
endmacro ()

# Install settings
if (WIN32)
    # FIXME: try to avoid install to a "bin" subfolder under windows...
    #        when fixed, don't forget to update the CI config file...
    set (BIN_INSTALL_DIR "") # seems useless, don't know why...
elseif (UNIX)
    if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX /usr)
    endif ()

    set (BIN_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}") # relative, usually "bin"
    pchaptool_convert_to_relative_path(BIN_INSTALL_DIR)
    set (LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}") # "lib" or "lib64"
    pchaptool_convert_to_relative_path(LIB_INSTALL_DIR)

    # install icon
    install (
        FILES assets/icons/app-icon.svg
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps"
        RENAME pineapple-chapter-tool.svg
    )

    # install shortcut
    install (
        FILES dist/net.blumia.pineapple-chapter-tool.desktop
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
    )
endif()

set (INSTALL_TARGETS_DEFAULT_ARGS
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR} COMPONENT Devel
)

install (
    TARGETS ${EXE_NAME}
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

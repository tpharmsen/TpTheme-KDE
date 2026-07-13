cmake_minimum_required(VERSION 3.16)
project(desklets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Standard dependencies needed for the pure C++ build
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)
find_package(KF6WindowSystem REQUIRED)
find_package(LayerShellQt REQUIRED)

# QTermWidget detection logic
find_package(PkgConfig QUIET)
set(QTERMWIDGET_FOUND FALSE)

if(PkgConfig_FOUND)
    pkg_check_modules(QTERMWIDGET_PC QUIET qtermwidget6)
    if(NOT QTERMWIDGET_PC_FOUND)
        pkg_check_modules(QTERMWIDGET_PC QUIET qtermwidget5)
    endif()
    if(QTERMWIDGET_PC_FOUND)
        set(QTERMWIDGET_FOUND TRUE)
        set(QTERMWIDGET_INCLUDE_DIRS ${QTERMWIDGET_PC_INCLUDE_DIRS})
        set(QTERMWIDGET_LIBRARIES ${QTERMWIDGET_PC_LIBRARIES})
        set(QTERMWIDGET_LIBRARY_DIRS ${QTERMWIDGET_PC_LIBRARY_DIRS})
        set(QTERMWIDGET_CFLAGS_OTHER ${QTERMWIDGET_PC_CFLAGS_OTHER})
    endif()
endif()

if(NOT QTERMWIDGET_FOUND)
    find_path(QTERMWIDGET_INCLUDE_DIR
        NAMES qtermwidget.h
        PATH_SUFFIXES qtermwidget6 qtermwidget5 qtermwidget
    )
    find_library(QTERMWIDGET_LIBRARY
        NAMES qtermwidget6 qtermwidget5 qtermwidget
    )
    if(QTERMWIDGET_INCLUDE_DIR AND QTERMWIDGET_LIBRARY)
        set(QTERMWIDGET_FOUND TRUE)
        set(QTERMWIDGET_INCLUDE_DIRS ${QTERMWIDGET_INCLUDE_DIR})
        set(QTERMWIDGET_LIBRARIES ${QTERMWIDGET_LIBRARY})
    endif()
endif()

# --- shared base class: masking + blur-behind, used by all widgets ---
add_library(shapedblur STATIC
    src/common/ShapedBlurWindow.cpp
    src/common/ShapedBlurWindow.h
)
target_link_libraries(shapedblur PUBLIC Qt6::Widgets KF6::WindowSystem)
target_include_directories(shapedblur PUBLIC src/common)
target_link_libraries(shapedblur PUBLIC Qt6::Widgets KF6::WindowSystem  LayerShellQt::Interface)

# --- clock widget ---
# Included clock.qrc to build the SVG bundle natively.
add_executable(clock-widget
    src/clock/main.cpp
    src/clock/ClockWidget.cpp
    src/clock/ClockWidget.h
    src/clock/clock.qrc
)
# Re-linked against shapedblur and added Qt6::Svg
target_link_libraries(clock-widget PRIVATE shapedblur Qt6::Svg)

# --- journalctl tail widget ---
add_executable(journal-widget
    src/journal/main.cpp
    src/journal/JournalWidget.cpp
    src/journal/JournalWidget.h
)
target_link_libraries(journal-widget PRIVATE shapedblur)



# --- journalctl tail widget 2---

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/journal2/sysbar.sh
               ${CMAKE_CURRENT_BINARY_DIR}/sysbar.sh
               COPYONLY)

add_executable(journal2-widget
    src/journal2/main.cpp
    src/journal2/Journal2Widget.cpp
    src/journal2/Journal2Widget.h
)
target_link_libraries(journal2-widget PRIVATE shapedblur)

# --- terminal widget ---
if(QTERMWIDGET_FOUND)
    add_executable(terminal-widget
        src/terminal/main.cpp
        src/terminal/TerminalWidget.cpp
        src/terminal/TerminalWidget.h
    )
    target_link_libraries(terminal-widget PRIVATE shapedblur ${QTERMWIDGET_LIBRARIES})
    target_include_directories(terminal-widget PRIVATE ${QTERMWIDGET_INCLUDE_DIRS})
    target_link_directories(terminal-widget PRIVATE ${QTERMWIDGET_LIBRARY_DIRS})
    target_compile_options(terminal-widget PRIVATE ${QTERMWIDGET_CFLAGS_OTHER})
endif()

install(TARGETS clock-widget journal-widget journal2-widget RUNTIME DESTINATION bin)
if(TARGET terminal-widget)
    install(TARGETS terminal-widget RUNTIME DESTINATION bin)
endif()

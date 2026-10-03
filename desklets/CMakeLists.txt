cmake_minimum_required(VERSION 3.16)
project(desklets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)
find_package(KF6WindowSystem REQUIRED)
find_package(LayerShellQt REQUIRED)

add_library(shapedblur STATIC
    src/common/ShapedBlurWindow.cpp
    src/common/ShapedBlurWindow.h
)
target_include_directories(shapedblur PUBLIC src/common)
target_link_libraries(shapedblur PUBLIC
    Qt6::Widgets
    KF6::WindowSystem
    LayerShellQt::Interface
)

add_executable(clock-widget
    src/clock/main.cpp
    src/clock/ClockWidget.cpp
    src/clock/ClockWidget.h
    src/clock/clock.qrc
)
target_link_libraries(clock-widget PRIVATE shapedblur Qt6::Svg)

add_executable(journal-widget
    src/journal/main.cpp
    src/journal/JournalWidget.cpp
    src/journal/JournalWidget.h
)
target_link_libraries(journal-widget PRIVATE shapedblur)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/system/sysbar.sh
    ${CMAKE_CURRENT_BINARY_DIR}/sysbar.sh
    COPYONLY
)

add_executable(system-widget
    src/system/main.cpp
    src/system/SystemWidget.cpp
    src/system/SystemWidget.h
)
target_link_libraries(system-widget PRIVATE shapedblur)

add_executable(quote-widget
    src/quote/main.cpp
    src/quote/QuoteWidget.cpp
    src/quote/QuoteWidget.h
)
target_link_libraries(quote-widget PRIVATE shapedblur)

add_executable(animation-widget
    src/animation/main.cpp
    src/animation/AnimationWidget.cpp
    src/animation/AnimationWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../svgs/signatureTP.svg
)
target_link_libraries(animation-widget PRIVATE shapedblur Qt6::Svg)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../svgs/signatureTP.svg
    ${CMAKE_CURRENT_BINARY_DIR}/signatureTP.svg
    COPYONLY
)

install(
    TARGETS clock-widget journal-widget system-widget quote-widget animation-widget
    RUNTIME DESTINATION bin
)
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/../svgs/signatureTP.svg
    DESTINATION bin
)

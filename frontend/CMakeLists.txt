cmake_minimum_required(VERSION 3.10)
project(MinecraftBotManager)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Use Qt5
find_package(Qt5 COMPONENTS Core Widgets Network WebSockets REQUIRED)

set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Network Qt5::WebSockets)
message(STATUS "Using Qt5 with WebSockets")

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
)

set(HEADERS
    src/mainwindow.h
)

# UI files
set(UI_FILES
    src/mainwindow.ui
)

# Resources - используем правильный путь
set(RESOURCES
    src/resources/resources.qrc
)

add_executable(MinecraftBotManager ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCES})

target_link_libraries(MinecraftBotManager
    ${QT_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

# Add threading support
find_package(Threads REQUIRED)
target_link_libraries(MinecraftBotManager Threads::Threads)

# Set RPATH
set_target_properties(MinecraftBotManager PROPERTIES
    INSTALL_RPATH "/usr/lib/x86_64-linux-gnu:/lib/x86_64-linux-gnu"
    BUILD_WITH_INSTALL_RPATH TRUE
)
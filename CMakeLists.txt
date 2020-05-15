cmake_minimum_required(VERSION 3.5)

project(TackenCounter VERSION 0.1.0)

add_definitions(-DCEREAL_THREAD_SAFE=1)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()


add_definitions(-std=c++11 -Wall -Wextra -pedantic -Wnon-virtual-dtor)

find_package(Cereal REQUIRED)

find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)
include_directories(${Qt5_INCLUDE_DIRS})
link_directories(${Qt5_LIBRARY_DIRS})

include_directories(src)

set(SRC
    src/PlayerStateDelegate.cpp
    src/LogModel.cpp
    src/StatsModel.cpp
    src/CounterWidget.cpp
    src/main.cpp

    ui/counterwidget.ui
)


add_executable(main ${SRC})
target_link_libraries(main Qt5::Core Qt5::Widgets Qt5::Charts)

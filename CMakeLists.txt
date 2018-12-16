cmake_minimum_required(VERSION 2.8)
project(Neopixel)

#version number
set (LightProject_VERSION_MAJOR 1)
set (LightProject_VERSION_MINOR 0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)            # Enable c++11 standard

set(SOURCE_FILES
    ws2812-rpi.cpp
    )

set(HEADER_FILES
    ws2812-rpi-defines.h
    ws2812-rpi.h
    )

add_library(${PROJECT_NAME} ${SOURCE_FILES})

cmake_minimum_required(VERSION 2.8)
project(Neopixel)

#version number
set (LightProject_VERSION_MAJOR 1)
set (LightProject_VERSION_MINOR 0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)            # Enable c++11 standard

set(CMAKE_CXX_FLAGS "-lrt")

set(SOURCE_FILES
    ws2812b.cpp
    ws2812bTest.cpp
    NeoPixel.cpp
    main.cpp
    )

set(HEADER_FILES
    ws2812-rpi-defines.h
    ws2812b.h
    ws2812bTest.h
    NeoPixel.h
    )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

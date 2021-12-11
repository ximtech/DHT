cmake_minimum_required(VERSION 3.20)
project(DHT VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(cmake/CPM.cmake)

CPMAddPackage(
        NAME DWTDelay
        GITHUB_REPOSITORY ximtech/DWTDelay
        GIT_TAG origin/main)

set(DHT_SENSOR_DIRECTORY
        ${DWT_DELAY_DIRECTORY}
        ${DHT_SOURCE_DIR}
        ${DHT_SOURCE_DIR}/include
        CACHE STRING "DHT sensor directories include to the main project")

set(DHT_SENSOR_SOURCES
        ${DWT_DELAY_SOURCES}
        ${DHT_SOURCE_DIR}/DHT.c
        ${DHT_SOURCE_DIR}/include/DHT.h
        CACHE STRING "DHT sensor sources include to the main project")
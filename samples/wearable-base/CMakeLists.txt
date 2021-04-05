cmake_minimum_required(VERSION 3.13.1)

# REMOVE THE BELOW WHEN CONTRIBUTING TO THE UPSTREAM ZEPHYR PROJECT
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../drivers/all-custom.cmake)

# Now perform the app build as normal
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(wearable-base)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
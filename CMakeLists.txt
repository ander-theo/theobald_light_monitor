cmake_minimum_required(VERSION 3.10)
project(LightMonitor VERSION 1.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STADARD_REQUIRED True)

configure_file(LightMonitor.h.in LightMonitor.h)

add_executable(LightMonitor light_monitor.cpp json.hpp httplib.h doctest.h)

target_include_directories(LightMonitor PUBLIC "${PROJECT_BINARY_DIR}")

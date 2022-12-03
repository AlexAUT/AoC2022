cmake_minimum_required(VERSION 3.22)

project(AoC2022)

add_library(common INTERFACE)

# Dependencies from vcpkg
find_package(Catch2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

target_link_libraries(common INTERFACE
  Catch2::Catch2
  Catch2::Catch2WithMain
  fmt::fmt
  range-v3
)
target_include_directories(common INTERFACE include)
set_target_properties(common PROPERTIES
  CXX_STANDARD 20
)

add_executable(day1 src/day1/main.cpp)
target_link_libraries(day1 PRIVATE common)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(day1)

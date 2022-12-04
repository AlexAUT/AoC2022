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
set_property(TARGET common PROPERTY CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

function(add_day_target DAY)
  add_executable(${DAY} src/${DAY}/main.cpp)
  target_link_libraries(${DAY} PRIVATE common)
  catch_discover_tests(${DAY})
  set_property(TARGET ${DAY} PROPERTY CXX_STANDARD 20)
endfunction()

add_day_target(day1)
add_day_target(day2)
add_day_target(day3)


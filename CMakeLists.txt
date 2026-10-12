cmake_minimum_required(VERSION 3.16)
project(foucault-pendulum
        VERSION 1.0.0
        DESCRIPTION "Foucault pendulum simulation with fourth-order Runge-Kutta"
        HOMEPAGE_URL "https://github.com/shahabahreini/foucault-pendulum"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Header-only core.
add_library(foucault INTERFACE)
add_library(foucault::foucault ALIAS foucault)
target_include_directories(foucault INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(foucault INTERFACE cxx_std_17)

add_executable(foucault-sim src/main.cpp)
target_link_libraries(foucault-sim PRIVATE foucault::foucault)
if(MSVC)
  target_compile_options(foucault-sim PRIVATE /W4)
else()
  target_compile_options(foucault-sim PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

install(TARGETS foucault-sim RUNTIME DESTINATION bin)
install(DIRECTORY include/foucault DESTINATION include)

option(FOUCAULT_BUILD_TESTS "Build the unit tests" ON)
if(FOUCAULT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

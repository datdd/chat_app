cmake_minimum_required(VERSION 3.10)
project(ChatApplication LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable per-target CMAKE_BUILD_TYPE for multi-config generators (like VS)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# For pthreads on non-Windows
if(NOT WIN32)
    find_package(Threads REQUIRED)
endif()

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
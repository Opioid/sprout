cmake_minimum_required(VERSION 3.1)
project(sprout)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()


if (LINUX) 
  # linux stuff here
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

include(base/base.cmake)
include(core/core.cmake)
include(extension/extension.cmake)
include(cli/cli.cmake)  

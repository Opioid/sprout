cmake_minimum_required(VERSION 3.1)
project(sprout)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# On Linux this should give -fPIC/fPIE
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()


if (LINUX) 
  # linux stuff here
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -W -Wall -fno-rtti")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
  #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

  #strip debug symbols
  #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
  if (CMAKE_BUILD_TYPE MATCHES RELEASE)
	set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
  endif (CMAKE_BUILD_TYPE MATCHES RELEASE)
else ()
  
endif ()

include(base/base.cmake)
include(core/core.cmake)
include(extension/extension.cmake)
include(cli/cli.cmake)  

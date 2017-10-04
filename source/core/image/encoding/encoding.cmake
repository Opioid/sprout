include("${CMAKE_CURRENT_LIST_DIR}/png/png.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/raw/raw.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/rgbe/rgbe.cmake")

target_sources(core
  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/encoding_srgb.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/encoding_srgb.hpp"
) 

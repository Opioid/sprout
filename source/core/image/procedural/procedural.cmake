include("${CMAKE_CURRENT_LIST_DIR}/flakes/flakes.cmake")

target_sources(core
  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/image_renderer.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/image_renderer.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/image_renderer.inl"
) 

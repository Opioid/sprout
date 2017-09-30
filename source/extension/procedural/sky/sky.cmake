include("${CMAKE_CURRENT_LIST_DIR}/hosek/hosek.cmake")

target_sources(extension
  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/sky_material_base.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/sky_material_base.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/sky_material.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/sky_material.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/sky_model.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/sky_model.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/sky_provider.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/sky_provider.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/sky.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/sky.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/sun_material.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/sun_material.hpp"
)

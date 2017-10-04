include("${CMAKE_CURRENT_LIST_DIR}/tonemapping/tonemapping.cmake")

target_sources(core
  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor_bloom.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor_bloom.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor_glare.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor_glare.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor_glare2.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor_glare2.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor_glare3.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor_glare3.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor_pipeline.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor_pipeline.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/postprocessor.hpp"
) 

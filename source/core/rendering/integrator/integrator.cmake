include("${CMAKE_CURRENT_LIST_DIR}/photon/photon.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/surface/surface.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/volume/volume.cmake")

target_sources(core
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/integrator_helper.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/integrator.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/integrator.hpp"
    ) 

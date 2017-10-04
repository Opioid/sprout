add_library(extension "") 

target_include_directories(extension PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
target_include_directories(extension PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/include/>)

target_link_libraries(extension PRIVATE base core)

include("${CMAKE_CURRENT_LIST_DIR}/procedural/procedural.cmake")

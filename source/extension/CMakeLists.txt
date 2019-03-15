add_library(extension "") 

target_include_directories(base PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(base PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/include/>)

target_link_libraries(extension PRIVATE base core)

add_subdirectory(procedural)

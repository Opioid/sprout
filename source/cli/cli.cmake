add_executable(cli "${CMAKE_CURRENT_LIST_DIR}/main.cpp") 

set_target_properties(cli PROPERTIES OUTPUT_NAME "sprout")

target_include_directories(cli PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
target_include_directories(cli PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../>)

target_link_libraries(cli PRIVATE base core extension)

include("${CMAKE_CURRENT_LIST_DIR}/controller/controller.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/options/options.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/server/server.cmake")

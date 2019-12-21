add_executable(capi-test "main.cpp")

set_target_properties(capi-test PROPERTIES OUTPUT_NAME "spcat")

target_include_directories(capi-test PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(capi-test PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>)

target_link_libraries(capi-test PRIVATE capi)


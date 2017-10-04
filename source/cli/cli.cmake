add_executable(cli "${CMAKE_CURRENT_LIST_DIR}/main.cpp") 

set_target_properties(cli PROPERTIES OUTPUT_NAME "sprout")

target_include_directories(cli PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
target_include_directories(cli PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../>)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

target_link_libraries(cli PRIVATE base core extension Threads::Threads)

if (WIN32)
	target_link_libraries(cli PRIVATE ws2_32)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/controller/controller.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/options/options.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/server/server.cmake")

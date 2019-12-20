add_library(capi SHARED "main.cpp")

set_target_properties(capi PROPERTIES OUTPUT_NAME "sprout")

target_include_directories(capi PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(capi PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

target_link_libraries(capi PRIVATE base core extension Threads::Threads)

if (WIN32)
	target_link_libraries(capi PRIVATE ws2_32)
else ()
    target_link_libraries(capi PRIVATE stdc++fs)
endif()

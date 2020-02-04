add_library(capi SHARED "capi.cpp" "sprout.h")

set_target_properties(capi PROPERTIES OUTPUT_NAME "sprout")

add_definitions(-DSU_LIBRARY_EXPORTS)

target_include_directories(capi PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(capi PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

target_link_libraries(capi PRIVATE base core extension Threads::Threads)

if (NOT WIN32)
    target_link_libraries(cli PRIVATE stdc++fs)
endif()

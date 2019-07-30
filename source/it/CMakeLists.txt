add_executable(it "main.cpp" "difference.hpp" "difference.cpp")

# set_target_properties(cli PROPERTIES OUTPUT_NAME "sprout")

target_include_directories(it PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(it PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

target_link_libraries(it PRIVATE base core Threads::Threads)

add_subdirectory(options)

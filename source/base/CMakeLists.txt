add_library(base "") 

target_include_directories(base PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(base PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/include/>)

add_subdirectory(atomic)
add_subdirectory(chrono)
add_subdirectory(debug)
add_subdirectory(encoding)
add_subdirectory(flags)
add_subdirectory(json)
add_subdirectory(math)
add_subdirectory(memory)
add_subdirectory(platform)
add_subdirectory(random)
add_subdirectory(spectrum)
add_subdirectory(string)
add_subdirectory(thread)

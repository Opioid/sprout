add_library(core "") 

target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>)
target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/include/>)

target_link_libraries(core PRIVATE base)

add_subdirectory(exporting)
add_subdirectory(file)
add_subdirectory(image)
add_subdirectory(logging)
add_subdirectory(progress)
add_subdirectory(rendering)
add_subdirectory(resource)
add_subdirectory(sampler)
add_subdirectory(scene)
add_subdirectory(take)
add_subdirectory(testing)

target_sources(core
	PRIVATE
	"../../thirdparty/include/miniz/miniz.cpp"
)

if (${ZSTD})
    target_sources(core
        PRIVATE
        "../../thirdparty/include/zstd/zstddeclib.cpp"
    )
endif()

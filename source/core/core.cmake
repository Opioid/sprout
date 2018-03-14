add_library(core "") 

target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../>)
target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/include/>)

target_link_libraries(core PRIVATE base)

include("${CMAKE_CURRENT_LIST_DIR}/baking/baking.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/exporting/exporting.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file/file.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/gzip/gzip.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/image/image.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/logging/logging.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/progress/progress.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/rendering/rendering.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/resource/resource.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/sampler/sampler.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/scene/scene.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/scripting/scripting.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/take/take.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/testing/testing.cmake")

target_sources(core
	PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/include/duktape/duktape.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/include/miniz/miniz.cpp"
)

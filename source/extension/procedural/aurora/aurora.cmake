target_sources(extension
	PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/aurora.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/aurora.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/aurora_provider.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/aurora_provider.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/aurora_spectrum.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/aurora_spectrum.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/volume_filter.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volume_filter.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/volume_rasterizer.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volume_rasterizer.hpp"
	)

include("${CMAKE_CURRENT_LIST_DIR}/filter/filter.cmake")

target_sources(core
	PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/clamp.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/clamp.inl"
	"${CMAKE_CURRENT_LIST_DIR}/filtered.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/filtered.inl"
	"${CMAKE_CURRENT_LIST_DIR}/opaque.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/opaque.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/sensor.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/sensor.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/transparent.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/transparent.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/unfiltered.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/unfiltered.inl"
	) 

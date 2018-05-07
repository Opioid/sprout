target_sources(core
	PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_atmosphere.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_atmosphere.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_density.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_density.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_density.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_grid.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_grid.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_height.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_height.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_homogeneous.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_homogeneous.hpp"	
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_material.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_material.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_octree.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_octree.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_octree_builder.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_octree_builder.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_sample.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/volumetric_sample.hpp"
	) 

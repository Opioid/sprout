include("${CMAKE_CURRENT_LIST_DIR}/distribution/distribution.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/filter/filter.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/fourier/fourier.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/function/function.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/sampling/sampling.cmake")

target_sources(base
	PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/aabb.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/aabb.inl"
	"${CMAKE_CURRENT_LIST_DIR}/exp.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/mapping.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/mapping.inl"
	"${CMAKE_CURRENT_LIST_DIR}/math.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/matrix.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/matrix.inl"
	"${CMAKE_CURRENT_LIST_DIR}/matrix3x3.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/matrix3x3.inl"
	"${CMAKE_CURRENT_LIST_DIR}/matrix4x4.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/matrix4x4.inl"
	"${CMAKE_CURRENT_LIST_DIR}/plane.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/plane.inl"
	"${CMAKE_CURRENT_LIST_DIR}/print.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/quaternion.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/quaternion.inl"
	"${CMAKE_CURRENT_LIST_DIR}/ray.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/ray.inl"
	"${CMAKE_CURRENT_LIST_DIR}/simd_aabb.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/simd_aabb.inl"
	"${CMAKE_CURRENT_LIST_DIR}/sincos.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/transformation.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/transformation.inl"
	"${CMAKE_CURRENT_LIST_DIR}/vector.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/vector.inl"
	"${CMAKE_CURRENT_LIST_DIR}/vector2.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/vector2.inl"
	"${CMAKE_CURRENT_LIST_DIR}/vector3.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/vector3.inl"
	"${CMAKE_CURRENT_LIST_DIR}/vector4.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/vector4.inl"
	"${CMAKE_CURRENT_LIST_DIR}/vector8.hpp"
	)

include("${CMAKE_CURRENT_LIST_DIR}/cloth/cloth.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/coating/coating.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/debug/debug.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/disney/disney.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/display/display.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/fresnel/fresnel.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/ggx/ggx.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/glass/glass.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/lambert/lambert.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/light/light.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/matte/matte.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/metal/metal.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/metallic_paint/metallic_paint.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/mix/mix.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/null/null.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/oren_nayar/oren_nayar.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/sky/sky.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/substitute/substitute.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/volumetric/volumetric.cmake")

target_sources(core
	PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/bxdf.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/collision_coefficients.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/material_attenuation.inl"
    "${CMAKE_CURRENT_LIST_DIR}/material_helper.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/material_print.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/material_provider.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/material_provider.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/material_sample_cache.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/material_sample_cache.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/material_sample_cache.inl"
    "${CMAKE_CURRENT_LIST_DIR}/material_sample_helper.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/material_sample.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/material_sample.inl"
    "${CMAKE_CURRENT_LIST_DIR}/material_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/material_test.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/material.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/material.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_cache.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_cache.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_settings.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_settings.hpp"
    ) 

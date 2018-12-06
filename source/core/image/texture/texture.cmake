include("${CMAKE_CURRENT_LIST_DIR}/sampler/sampler.cmake")

target_sources(core
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/texture_adapter.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_adapter.inl"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_1_unorm.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_1_unorm.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_2_snorm.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_2_snorm.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_2_unorm.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_2_unorm.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_3_snorm.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_3_snorm.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_3_srgb.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_3_srgb.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_3_unorm.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_byte_3_unorm.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_encoding.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_encoding.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_float_1.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_float_1.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_float_2.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_float_2.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_float_3.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_float_3.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_provider.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_provider.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_test.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture_types.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/texture.hpp"
    ) 

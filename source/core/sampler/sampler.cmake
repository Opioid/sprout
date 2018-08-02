target_sources(core
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/camera_sample.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_golden_ratio.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_golden_ratio.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_hammersley.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_hammersley.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_ld.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_ld.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_random.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_random.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_test.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_test.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_uniform.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler_uniform.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sampler.hpp"
) 

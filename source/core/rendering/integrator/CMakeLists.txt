add_subdirectory(particle)
add_subdirectory(surface)
add_subdirectory(volume)

target_sources(core
    PRIVATE
    "integrator_helper.hpp"
    "integrator.cpp"
    "integrator.inl"
    "integrator.hpp"
    ) 

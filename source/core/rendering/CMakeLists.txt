add_subdirectory(integrator)
add_subdirectory(postprocessor)
add_subdirectory(sensor)

target_sources(core
    PRIVATE
    "rendering_driver.cpp"
    "rendering_driver.hpp"
    "rendering_worker.cpp"
    "rendering_worker.hpp"
    "rendering_worker.inl"
    "tile_queue.cpp"
    "tile_queue.hpp"
    )

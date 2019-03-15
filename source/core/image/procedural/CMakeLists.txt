add_subdirectory(flakes)

target_sources(core
  PRIVATE
  "image_renderer.cpp"
  "image_renderer.hpp"
  "image_renderer.inl"
) 

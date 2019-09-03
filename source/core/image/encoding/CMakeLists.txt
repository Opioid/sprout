add_subdirectory("exr")
add_subdirectory("json")
add_subdirectory("png")
add_subdirectory("raw")
add_subdirectory("rgbe")
add_subdirectory("sub")

target_sources(core
    PRIVATE
    "encoding_srgb.cpp"
    "encoding_srgb.hpp"
    )

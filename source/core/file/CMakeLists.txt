target_sources(core
    PRIVATE
    "file_system.cpp"
    "file_system.hpp"
    "file.cpp"
    "file.hpp"
    "gzip_read_stream.cpp"
    "gzip_read_stream.hpp"
    "read_stream.hpp"
    "read_stream.inl"
    )   

if (${ZSTD})
    target_sources(core
        PRIVATE
        "zstd_read_stream.cpp"
        "zstd_read_stream.hpp"
        )
endif()

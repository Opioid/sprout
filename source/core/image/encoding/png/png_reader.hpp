#ifndef SU_CORE_IMAGE_ENCODING_PNG_READER_HPP
#define SU_CORE_IMAGE_ENCODING_PNG_READER_HPP

#include "image/channels.hpp"
#include "miniz/miniz.h"

#include <cstdint>
#include <iosfwd>

namespace image {

class Image;

namespace encoding::png {

class Reader {
  public:
    Image* read(std::istream& stream, Swizzle swizzle, bool invert);

    Image* create_from_buffer(Swizzle swizzle, bool invert) const;

    struct Chunk {
        ~Chunk();

        void allocate();

        uint32_t length   = 0;
        uint32_t capacity = 0;

        uint8_t* data = nullptr;
    };

    enum class Filter { None, Sub, Up, Average, Paeth };

    struct Info {
        Info();

        ~Info();

        bool allocate();

        // header
        int32_t width  = 0;
        int32_t height = 0;

        int32_t num_channels    = 0;
        int32_t bytes_per_pixel = 0;

        // parsing state
        Filter  current_filter;
        bool    filter_byte;
        int32_t current_byte;
        int32_t current_byte_total;

        uint32_t capacity = 0;

        uint8_t* buffer            = nullptr;
        uint8_t* current_row_data  = nullptr;
        uint8_t* previous_row_data = nullptr;

        // miniz
        mz_stream stream;
    };

  private:
    Chunk chunk_;

    Info info_;
};

}  // namespace encoding::png
}  // namespace image

#endif

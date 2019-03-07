#ifndef SU_CORE_IMAGE_ENCODING_PNG_READER_HPP
#define SU_CORE_IMAGE_ENCODING_PNG_READER_HPP

// based on
// https://github.com/jansol/LuPng

#include <array>
#include <cstdint>
#include <iosfwd>
#include "image/channels.hpp"
#include "miniz/miniz.hpp"

namespace image {

class Image;

namespace encoding::png {

class Reader {
  public:
    static Image* read(std::istream& stream, Channels channels, int32_t num_elements, bool swap_xy,
                       bool invert);

  private:
    struct Chunk {
        ~Chunk() noexcept;

        uint32_t length   = 0;
        uint32_t capacity = 0;
        uint8_t* type     = nullptr;
        uint8_t* data;
        uint32_t crc;
    };

    enum class Color_type {
        Grayscale       = 0,
        Truecolor       = 2,
        Palleted        = 3,
        Grayscale_alpha = 4,
        Truecolor_alpha = 6
    };

    enum class Filter { None, Sub, Up, Average, Paeth };

    struct Info {
        ~Info() noexcept;

        // header
        int32_t width  = 0;
        int32_t height = 0;

        int32_t num_channels    = 0;
        int32_t bytes_per_pixel = 0;

        uint8_t* buffer = nullptr;

        // parsing state
        Filter   current_filter;
        bool     filter_byte;
        uint32_t current_byte;
        uint32_t current_byte_total;

        uint8_t* current_row_data  = nullptr;
        uint8_t* previous_row_data = nullptr;

        // miniz
        mz_stream stream;
    };

    static Image* create_image(const Info& info, Channels channels, int32_t num_elements,
                               bool swap_xy, bool invert);

    static void read_chunk(std::istream& stream, Chunk& chunk);

    static bool handle_chunk(const Chunk& chunk, Info& info);

    static bool parse_header(const Chunk& chunk, Info& info);

    static bool parse_lte(const Chunk& chunk, Info& info);

    static bool parse_data(const Chunk& chunk, Info& info);

    static uint8_t filter(uint8_t byte, Filter filter, const Info& info);

    static uint8_t raw(int column, const Info& info);
    static uint8_t prior(int column, const Info& info);

    static uint8_t average(uint8_t a, uint8_t b);
    static uint8_t paeth_predictor(uint8_t a, uint8_t b, uint8_t c);

    static uint32_t byteswap(uint32_t v);

    static uint32_t const Signature_size = 8;

    static const std::array<uint8_t, Signature_size> Signature;
};

}  // namespace encoding::png
}  // namespace image

#endif

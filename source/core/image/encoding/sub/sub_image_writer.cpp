#include "sub_image_writer.hpp"
#include <fstream>
#include "base/memory/bitfield.inl"
#include "base/string/string.hpp"
#include "image/image.hpp"
#include "image/typed_image.inl"
#include "image/typed_image_fwd.hpp"

#include <iostream>

namespace image::encoding::sub {

static void newline(std::ostream& stream, uint32_t num_tabs) {
    stream << std::endl;

    for (uint32_t i = 0; i < num_tabs; ++i) {
        stream << '\t';
    }
}

static void binary_tag(std::ostream& stream, size_t offset, size_t size) {
    stream << "\"binary\":{\"offset\":" << offset << ",\"size\":" << size << "}";
}

static std::string image_type_string(Image::Type type) {
    switch (type) {
        case Image::Type::Byte1:
            return "Byte1";
        case Image::Type::Float1:
            return "Float1";
        default:
            return "Undefined";
    }
}

static std::string image_type_encoding(Image::Type type) {
    switch (type) {
        case Image::Type::Byte1:
            return "UInt8";
        case Image::Type::Float1:
            return "Float32";
        default:
            return "Undefined";
    }
}

static uint32_t image_type_bytes_per_pixel(Image::Type type) {
    switch (type) {
        case Image::Type::Byte1:
            return 1;
        case Image::Type::Float1:
            return 4;
        default:
            return 0;
    }
}

void Writer::write(std::string const& filename, Image const& image) {
    std::string const out_name = string::extract_filename(filename) + ".sub";

    std::cout << "Export " << out_name << std::endl;

    std::ofstream stream(out_name, std::ios::binary);

    if (!stream) {
        return;
    }

    const char header[] = "SUB\000";
    stream.write(header, sizeof(char) * 4);

    std::stringstream jstream;

    newline(jstream, 0);
    jstream << "{";

    newline(jstream, 1);
    jstream << "\"image\":{";

    auto const& description = image.description();

    newline(jstream, 2);
    jstream << "\"description\":{";

    newline(jstream, 3);
    jstream << "\"type\":\"" << image_type_string(image.description().type) << "\",";

    int3 const& d = description.dimensions;
    newline(jstream, 3);
    jstream << "\"dimensions\":[";
    jstream << d[0] << "," << d[1] << "," << d[2] << "],";

    newline(jstream, 3);
    jstream << "\"num_elements\":" << description.num_elements;

    // close description
    newline(jstream, 2);
    jstream << "},";

    static bool constexpr Bitfield = true;

    if (Bitfield) {
        newline(jstream, 2);
        jstream << "\"topology\":{";

        memory::Bitfield field(description.num_pixels());

        uint64_t const pixel_offset = field.num_bytes();

        newline(jstream, 3);
        binary_tag(jstream, 0, pixel_offset);

        // close topology
        newline(jstream, 2);
        jstream << "},";

        field.clear();
        uint64_t num_active_pixels = 0;

        if (Image::Type::Byte1 == description.type) {
            image::Byte1 const& typed = *static_cast<image::Byte1 const*>(&image);

            for (uint64_t i = 0, len = description.num_pixels(); i < len; ++i) {
                uint8_t const density = typed.data()[i];

                if (density > 0) {
                    field.set(i, true);
                    ++num_active_pixels;
                }
            }
        } else /*if (Image::Type::Float1 == description.type)*/ {
            image::Float1 const& typed = *static_cast<image::Float1 const*>(&image);

            for (uint64_t i = 0, len = description.num_pixels(); i < len; ++i) {
                float const density = typed.data()[i];

                if (density > 0.f) {
                    field.set(i, true);
                    ++num_active_pixels;
                }
            }
        }

        newline(jstream, 2);
        jstream << "\"pixels\":{";

        uint64_t const pixels_size = num_active_pixels *
                                     image_type_bytes_per_pixel(description.type);

        newline(jstream, 3);
        binary_tag(jstream, pixel_offset, pixels_size);
        jstream << ",";

        newline(jstream, 3);
        jstream << "\"encoding\":\"Float32\"";

        // close pixels
        newline(jstream, 2);
        jstream << "}";

        // close image
        newline(jstream, 1);
        jstream << "}";

        // close start
        newline(jstream, 0);
        jstream << "}";

        newline(jstream, 0);

        std::string const json_string = jstream.str();
        uint64_t const    json_size   = json_string.size() - 1;

        stream.write(reinterpret_cast<char const*>(&json_size), sizeof(uint64_t));
        stream.write(reinterpret_cast<char const*>(json_string.data()),
                     static_cast<std::streamsize>(json_size * sizeof(char)));

        stream.write(reinterpret_cast<char const*>(field.data()),
                     static_cast<std::streamsize>(field.num_bytes()));

        if (Image::Type::Byte1 == description.type) {
            image::Byte1 const& typed = *static_cast<image::Byte1 const*>(&image);

            for (uint64_t i = 0, len = description.num_pixels(); i < len; ++i) {
                uint8_t const density = typed.data()[i];

                if (density > 0) {
                    stream.write(reinterpret_cast<char const*>(&density), sizeof(uint8_t));
                }
            }
        } else /*if (Image::Type::Float1 == description.type)*/ {
            image::Float1 const& typed = *static_cast<image::Float1 const*>(&image);

            for (uint64_t i = 0, len = description.num_pixels(); i < len; ++i) {
                float const density = typed.data()[i];

                if (density > 0.f) {
                    stream.write(reinterpret_cast<char const*>(&density), sizeof(float));
                }
            }
        }
    } else {
        newline(jstream, 2);
        jstream << "\"pixels\":{";

        uint64_t const pixels_size = description.num_pixels() *
                                     image_type_bytes_per_pixel(description.type);

        newline(jstream, 3);
        binary_tag(jstream, 0, pixels_size);
        jstream << ",";

        newline(jstream, 3);
        jstream << "\"encoding\":\"" << image_type_encoding(description.type) << "\"";

        // close pixels
        newline(jstream, 2);
        jstream << "}";

        // close image
        newline(jstream, 1);
        jstream << "}";

        // close start
        newline(jstream, 0);
        jstream << "}";

        newline(jstream, 0);

        std::string const json_string = jstream.str();
        uint64_t const    json_size   = json_string.size() - 1;

        stream.write(reinterpret_cast<char const*>(&json_size), sizeof(uint64_t));
        stream.write(reinterpret_cast<char const*>(json_string.data()),
                     static_cast<std::streamsize>(json_size * sizeof(char)));

        if (Image::Type::Byte1 == description.type) {
            image::Byte1 const& typed = *static_cast<image::Byte1 const*>(&image);

            stream.write(reinterpret_cast<char const*>(typed.data()),
                         static_cast<std::streamsize>(pixels_size));
        } else /*if (Image::Type::Float1 == description.type)*/ {
            image::Float1 const& typed = *static_cast<image::Float1 const*>(&image);

            stream.write(reinterpret_cast<char const*>(typed.data()),
                         static_cast<std::streamsize>(pixels_size));
        }
    }
}

}  // namespace image::encoding::sub

#include "sub_image_writer.hpp"
#include <fstream>
#include "base/string/string.hpp"
#include "image/image.hpp"
#include "image/typed_image.inl"
#include "image/typed_image_fwd.hpp"

#include <iostream>

namespace image::encoding::sub {

void newline(std::ostream& stream, uint32_t num_tabs) {
    stream << std::endl;

    for (uint32_t i = 0; i < num_tabs; ++i) {
        stream << '\t';
    }
}

void binary_tag(std::ostream& stream, size_t offset, size_t size) {
    stream << "\"binary\":{\"offset\":" << offset << ",\"size\":" << size << "}";
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
    jstream << "\"type\":\"Float1\",";

    int3 const& d = description.dimensions;
    newline(jstream, 3);
    jstream << "\"dimensions\":[";
    jstream << d[0] << "," << d[1] << "," << d[2] << "],";

    newline(jstream, 3);
    jstream << "\"num_elements\":" << description.num_elements;

    // close description
    newline(jstream, 2);
    jstream << "},";

    newline(jstream, 2);
    jstream << "\"pixels\":{";

    size_t const pixels_size = static_cast<size_t>(d[0] * d[1] * d[2]) * sizeof(float);

    newline(jstream, 3);
    binary_tag(jstream, 0, pixels_size);
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
    stream.write(reinterpret_cast<char const*>(json_string.data()), json_size * sizeof(char));

    image::Float1 const& typed = *static_cast<image::Float1 const*>(&image);

    stream.write(reinterpret_cast<char const*>(typed.data()), pixels_size);
}

}  // namespace image::encoding::sub

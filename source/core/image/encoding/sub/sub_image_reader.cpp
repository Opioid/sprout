#include "sub_image_reader.hpp"
#include <fstream>
#include "image/image.hpp"
#include "image/typed_image.inl"
#include "image/typed_image_fwd.hpp"
#include "json/json.hpp"

namespace image::encoding::sub {

std::shared_ptr<Image> Reader::read(std::istream& stream) {
    stream.seekg(4);

    uint64_t json_size = 0;
    stream.read(reinterpret_cast<char*>(&json_size), sizeof(uint64_t));

    char* json_string = new char[json_size + 1];
    stream.read(json_string, json_size * sizeof(char));
    json_string[json_size] = 0;

    auto const root = json::parse_insitu(json_string);

    auto const image_node = root->FindMember("image");
    if (root->MemberEnd() == image_node) {
        throw std::runtime_error("No image declaration");
    }

    auto const description_node = image_node->value.FindMember("description");
    if (image_node->value.MemberEnd() == description_node) {
        throw std::runtime_error("No image description");
    }

    int3 const dimensions = ::json::read_int3(description_node->value, "dimensions", int3(-1));

    if (-1 == dimensions[0]) {
        throw std::runtime_error("Invalid dimensions");
    }

    Image::Description description(Image::Type::Float1, dimensions);

    auto image = std::make_shared<Float1>(description);

    //    auto const topology_node = image_node->value.FindMember("topology");
    //    if (image_node->value.MemberEnd() == description_node) {
    //        throw std::runtime_error("No image description");
    //    }

    auto const pixels_node = image_node->value.FindMember("pixels");
    if (image_node->value.MemberEnd() == pixels_node) {
        throw std::runtime_error("No pixels");
    }

    size_t pixels_offset = 0;
    size_t pixels_size   = 0;

    for (auto const& pn : pixels_node->value.GetObject()) {
        if ("binary" == pn.name) {
            pixels_offset = json::read_uint(pn.value, "offset");
            pixels_size   = json::read_uint(pn.value, "size");
        } else if ("encoding" == pn.name) {
            //            if ("UInt16" == json::read_string(in.value)) {
            //                index_bytes = 2;
            //            } else {
            //                index_bytes = 4;
            //            }
        }
    }

    delete[] json_string;

    uint64_t const binary_start = json_size + 4u + sizeof(uint64_t);

    stream.seekg(binary_start + pixels_offset);
    stream.read(reinterpret_cast<char*>(image->data()), pixels_size);

    return image;
}

}  // namespace image::encoding::sub

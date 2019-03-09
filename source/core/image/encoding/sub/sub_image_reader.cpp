#include "sub_image_reader.hpp"
#include <fstream>
#include <vector>
#include "base/memory/bitfield.inl"
#include "base/string/string.hpp"
#include "image/image.hpp"
#include "image/typed_image.hpp"
#include "json/json.hpp"
#include "logging/logging.hpp"

namespace image::encoding::sub {

static Image::Type read_image_type(json::Value const& value) noexcept {
    if (auto const node = value.FindMember("type"); value.MemberEnd() != node) {
        if ("Byte1" == node->value) {
            return Image::Type::Byte1;
        } else if ("Float1" == node->value) {
            return Image::Type::Float1;
        }
    }

    return Image::Type::Undefined;
}

Image* Reader::read(std::istream& stream) noexcept {
    stream.seekg(4);

    uint64_t json_size = 0;
    stream.read(reinterpret_cast<char*>(&json_size), sizeof(uint64_t));

    std::vector<char> json_string(json_size + 1);
    stream.read(json_string.data(), static_cast<std::streamsize>(json_size * sizeof(char)));
    json_string[json_size] = 0;

    std::string error;
    auto const  root = json::parse_insitu(json_string.data(), error);
    if (!root) {
        logging::push_error(error);
        return nullptr;
    }

    auto const image_node = root->FindMember("image");
    if (root->MemberEnd() == image_node) {
        logging::push_error("No image declaration.");
        return nullptr;
    }

    auto const description_node = image_node->value.FindMember("description");
    if (image_node->value.MemberEnd() == description_node) {
        logging::push_error("No image description.");
        return nullptr;
    }

    int3 const dimensions = ::json::read_int3(description_node->value, "dimensions", int3(-1));

    if (-1 == dimensions[0]) {
        logging::push_error("Invalid dimensions.");
        return nullptr;
    }

    Image::Type const type = read_image_type(description_node->value);

    if (Image::Type::Undefined == type) {
        logging::push_error("Undefined image type.");
        return nullptr;
    }

    auto const topology_node = image_node->value.FindMember("topology");

    auto const pixels_node = image_node->value.FindMember("pixels");
    if (image_node->value.MemberEnd() == pixels_node) {
        logging::push_error("No pixels.");
        return nullptr;
    }

    uint64_t pixels_offset = 0;
    uint64_t pixels_size   = 0;

    for (auto const& pn : pixels_node->value.GetObject()) {
        if ("binary" == pn.name) {
            pixels_offset = json::read_uint64(pn.value, "offset");
            pixels_size   = json::read_uint64(pn.value, "size");
        } else if ("encoding" == pn.name) {
            //            if ("UInt16" == json::read_string(in.value)) {
            //                index_bytes = 2;
            //            } else {
            //                index_bytes = 4;
            //            }
        }
    }

    uint64_t const binary_start = json_size + 4u + sizeof(uint64_t);

    if (topology_node->value.MemberEnd() != topology_node) {
        uint64_t topology_offset = 0;
        uint64_t topology_size   = 0;

        for (auto const& tn : topology_node->value.GetObject()) {
            if ("binary" == tn.name) {
                topology_offset = json::read_uint64(tn.value, "offset");
                topology_size   = json::read_uint64(tn.value, "size");
                break;
            }
        }

        if (0 == topology_size) {
            logging::push_error("Empty topology.");
            return nullptr;
        }

        if (Image::Type::Byte1 == type) {
            Image::Description description(Image::Type::Byte1, dimensions);

            memory::Bitfield field(description.num_pixels());

            stream.seekg(static_cast<std::streamoff>(binary_start + topology_offset));
            stream.read(reinterpret_cast<char*>(field.data()),
                        static_cast<std::streamsize>(field.num_bytes()));

            stream.seekg(static_cast<std::streamoff>(binary_start + pixels_offset));

            auto image = new Byte1(description);

            uint8_t* data = image->data();

            for (uint64_t i = 0, len = description.num_pixels(); i < len; ++i) {
                if (field.get(i)) {
                    uint8_t density;
                    stream.read(reinterpret_cast<char*>(&density), sizeof(uint8_t));
                    data[i] = density;
                } else {
                    data[i] = 0;
                }
            }

            return image;
        } else /*if (Image::Type::Float1 == type)*/ {
            static bool constexpr sparse = true;

            Image::Description description(
                sparse ? Image::Type::Float1_sparse : Image::Type::Float1, dimensions);

            memory::Bitfield field(description.num_pixels());

            stream.seekg(static_cast<std::streamoff>(binary_start + topology_offset));
            stream.read(reinterpret_cast<char*>(field.data()),
                        static_cast<std::streamsize>(field.num_bytes()));

            stream.seekg(static_cast<std::streamoff>(binary_start + pixels_offset));

            if (sparse) {
                auto image = new Float1_sparse(description);

                for (uint64_t i = 0, len = description.num_pixels(); i < len; ++i) {
                    if (field.get(i)) {
                        float density;
                        stream.read(reinterpret_cast<char*>(&density), sizeof(float));

                        image->store_sequentially(static_cast<int64_t>(i), density);
                    }
                }

                return image;
            } else {
                auto image = new Float1(description);

                float* data = image->data();

                for (uint64_t i = 0, len = description.num_pixels(); i < len; ++i) {
                    if (field.get(i)) {
                        float density;
                        stream.read(reinterpret_cast<char*>(&density), sizeof(float));
                        data[i] = density;
                    } else {
                        data[i] = 0.f;
                    }
                }

                return image;
            }
        }
    } else {
        stream.seekg(static_cast<std::streamoff>(binary_start + pixels_offset));

        if (Image::Type::Byte1 == type) {
            Image::Description description(Image::Type::Byte1, dimensions);

            auto image = new Byte1(description);

            stream.read(reinterpret_cast<char*>(image->data()),
                        static_cast<std::streamsize>(pixels_size));

            return image;
        } else /*if (Image::Type::Float1 == type)*/ {
            Image::Description description(Image::Type::Float1, dimensions);

            auto image = new Float1(description);

            stream.read(reinterpret_cast<char*>(image->data()),
                        static_cast<std::streamsize>(pixels_size));

            return image;
        }
    }
}

}  // namespace image::encoding::sub

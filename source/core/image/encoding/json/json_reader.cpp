#include "json_reader.hpp"
#include <istream>
#include <string>
#include "base/encoding/encoding.inl"
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.inl"

#include <fstream>

namespace image::encoding::json {

std::shared_ptr<Image> Reader::read(std::istream& stream) const {
    auto root = ::json::parse(stream);

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

    auto const data_node = image_node->value.FindMember("data");
    if (image_node->value.MemberEnd() == data_node) {
        throw std::runtime_error("No image data");
    }

    bool const is_byte1 = "Byte1" == ::json::read_string(description_node->value, "type");

    if (is_byte1) {
        Image::Description description(Image::Type::Byte1, dimensions);

        auto volume = std::make_shared<Byte1>(description);

        int32_t i = 0;
        for (auto const& v : data_node->value.GetArray()) {
            volume->at(i++) = static_cast<uint8_t>(v.GetUint());
        }

        return volume;
    }

    static bool constexpr export_unorm = false;

    if (export_unorm) {
        std::ofstream ostream("volume.json");

        ostream << "{" << std::endl;
        ostream << "\t\"image\": {" << std::endl;
        ostream << "\t\t\"description\": {" << std::endl;
        ostream << "\t\t\t\"type\": \"Byte1\"," << std::endl;
        ostream << "\t\t\t\"dimensions\": [" << dimensions[0] << ", " << dimensions[1] << ", "
                << dimensions[2] << "]" << std::endl;
        ostream << "\t\t}, " << std::endl;
        ostream << "\t\t\"data\": [" << std::endl;

        Image::Description description(Image::Type::Byte1, dimensions);

        auto volume = std::make_shared<Byte1>(description);

        int32_t const len = volume->volume();

        int32_t i = 0;
        for (auto const& v : data_node->value.GetArray()) {
            volume->at(i++) = ::encoding::float_to_unorm(v.GetFloat());

            ostream << static_cast<uint32_t>(::encoding::float_to_unorm(v.GetFloat()));

            if (i < len) {
                ostream << ",";
            }

            if (0 == i % 32) {
                ostream << std::endl;
            }
        }

        ostream << "\t\t]" << std::endl;
        ostream << "\t} " << std::endl;
        ostream << "} " << std::endl;

        return volume;
    } else {
        Image::Description description(Image::Type::Float1, dimensions);

        auto volume = std::make_shared<Float1>(description);

        int32_t i = 0;
        for (auto const& v : data_node->value.GetArray()) {
            volume->at(i++) = v.GetFloat();
        }

        return volume;
    }
}

}  // namespace image::encoding::json

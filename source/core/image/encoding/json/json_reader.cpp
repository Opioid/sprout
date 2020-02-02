#include "json_reader.hpp"
#include <fstream>
#include <istream>
#include <string>
#include "base/encoding/encoding.inl"
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "image/encoding/sub/sub_image_writer.hpp"
#include "image/typed_image.hpp"
#include "image_json_handler.hpp"
#include "rapidjson/istreamwrapper.h"

namespace image::encoding::json {

Image* Reader::read(std::istream& stream, std::string const& filename) {
    Json_handler handler;

    rapidjson::IStreamWrapper json_stream(stream);

    rapidjson::Reader reader;

    auto const result = reader.Parse(json_stream, handler);

    if (!result) {
        return nullptr;
    }

    sub::Writer::write(filename, *handler.image());

    return handler.image();

    /*
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
            volume->store(i++, uint8_t(v.GetUint()));
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
            volume->store(i++, ::encoding::float_to_unorm(v.GetFloat()));

            ostream << uint32_t(::encoding::float_to_unorm(v.GetFloat()));

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
            volume->store(i++, v.GetFloat());
        }

        return volume;
    }
    */
}

}  // namespace image::encoding::json

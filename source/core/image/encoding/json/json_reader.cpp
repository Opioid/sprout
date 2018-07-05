#include "json_reader.hpp"
#include <istream>
#include <string>
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.inl"

namespace image::encoding::json {

std::shared_ptr<Image> Reader::read(std::istream& stream, std::string& error) const {
    auto root = ::json::parse(stream, error);
    if (!root) {
        return nullptr;
    }

    auto const image_node = root->FindMember("image");
    if (root->MemberEnd() == image_node) {
        error = "No image declaration";
        return nullptr;
    }

    auto const description_node = image_node->value.FindMember("description");
    if (image_node->value.MemberEnd() == description_node) {
        error = "No image description";
        return nullptr;
    }

    int3 const dimensions = ::json::read_int3(description_node->value, "dimensions", int3(-1));

    if (-1 == dimensions[0]) {
        error = "Invalid dimensions";
        return nullptr;
    }

    auto const data_node = image_node->value.FindMember("data");
    if (image_node->value.MemberEnd() == data_node) {
        error = "No image data";
        return nullptr;
    }

    Image::Description description(Image::Type::Float1, dimensions);

    auto volume = std::make_shared<Float1>(description);

    int32_t i = 0;
    for (auto const& v : data_node->value.GetArray()) {
        volume->at(i++) = v.GetFloat();
    }

    return volume;
}

}  // namespace image::encoding::json

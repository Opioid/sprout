#include "image_json_handler.hpp"
#include "base/math/vector3.inl"
#include "image/typed_image.hpp"

#include <string_view>

namespace image::encoding::json {

Byte1_handler::Byte1_handler(Byte1& image) : image_(image) {}

void Byte1_handler::add(uint32_t value) {
    image_.store(current_texel_++, uint8_t(value));
}

void Byte1_handler::add(float value) {
    image_.store(current_texel_++, uint8_t(value));
}

Float1_handler::Float1_handler(Float1& image) : image_(image) {}

void Float1_handler::add(uint32_t value) {
    image_.store(current_texel_++, float(value));
}

void Float1_handler::add(float value) {
    image_.store(current_texel_++, value);
}

Image_handler::~Image_handler() = default;

Json_handler::Json_handler() {
    clear();
}

Json_handler::~Json_handler() {
    delete image_handler_;
}

void Json_handler::clear() {
    object_level_ = 0;
    top_object_   = Object::Undefined;

    expected_number_ = Number::Undefined;
    expected_string_ = String_type::Undefined;
    expected_object_ = Object::Undefined;

    type_ = Image::Type::Float1;

    dimensions_component_ = 0;

    current_texel_ = 0;

    image_handler_ = nullptr;
}

Image* Json_handler::image() const {
    return image_;
}

bool Json_handler::Null() {
    return true;
}

bool Json_handler::Bool(bool /*b*/) {
    return true;
}

bool Json_handler::Int(int /*i*/) {
    // handle_vertex(float(i));

    return true;
}

bool Json_handler::Uint(unsigned i) {
    switch (expected_number_) {
        case Number::Dimensions:
            dimensions_[dimensions_component_++] = static_cast<int32_t>(i);
            break;
        case Number::Data:
            image_handler_->add(i);
            break;
        default:
            break;
    }

    return true;
}

bool Json_handler::Int64(int64_t /*i*/) {
    return true;
}

bool Json_handler::Uint64(uint64_t /*i*/) {
    return true;
}

bool Json_handler::Double(double d) {
    image_handler_->add(float(d));

    return true;
}

bool Json_handler::RawNumber(char const* /*str*/, rapidjson::SizeType /*length*/, bool /*copy*/) {
    return true;
}

bool Json_handler::String(char const* str, rapidjson::SizeType /*length*/, bool /*copy*/) {
    if (String_type::Type == expected_string_) {
        std::string_view const type(str);

        if ("Byte1" == type) {
            type_ = Image::Type::Byte1;
        } else if ("Float1" == type) {
            type_ = Image::Type::Float1;
        }
    }

    return true;
}

bool Json_handler::StartObject() {
    ++object_level_;

    //    switch (expected_object_) {
    //        default:
    //            break;
    //    }

    return true;
}

bool Json_handler::Key(char const* str, rapidjson::SizeType /*length*/, bool /*copy*/) {
    std::string_view const name(str);

    if (1 == object_level_) {
        if ("image" == name) {
            top_object_ = Object::Image;
            return true;
        }
    }

    if (Object::Image == top_object_) {
        if ("description" == name) {
            expected_object_ = Object::Description;
        } else if ("type" == name) {
            expected_string_ = String_type::Type;
        } else if ("dimensions" == name) {
            expected_number_ = Number::Dimensions;
        } else if ("data" == name) {
            if (!image_handler_) {
                //    throw std::runtime_error("Image description could not be parsed");
                return false;
            }

            expected_number_ = Number::Data;
        }

        return true;
    }

    return false;
}

bool Json_handler::EndObject(size_t /*memberCount*/) {
    if (Object::Description == expected_object_) {
        expected_object_ = Object::Undefined;

        if (Image::Type::Byte1 == type_) {
            Description description(dimensions_, int3(0));

            image_ = new Image(Byte1(description));

            image_handler_ = new Byte1_handler(image_->byte1());
        } else if (Image::Type::Float1 == type_) {
            Description description(dimensions_, int3(0));

            image_ = new Image(Float1(description));

            image_handler_ = new Float1_handler(image_->float1());
        }
    }

    --object_level_;
    return true;
}

bool Json_handler::StartArray() {
    return true;
}

bool Json_handler::EndArray(size_t /*elementCount*/) {
    return true;
}

}  // namespace image::encoding::json

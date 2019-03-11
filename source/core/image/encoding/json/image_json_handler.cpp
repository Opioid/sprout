#include "image_json_handler.hpp"
#include <stdexcept>
#include "base/math/vector3.inl"
#include "image/typed_image.hpp"

namespace image::encoding::json {

class Byte1_handler : public Json_handler::Image_handler {
  public:
    Byte1_handler(Image& image) : image_(static_cast<Byte1&>(image)) {}

    void add(uint32_t value) override final {
        image_.store(current_texel_++, static_cast<uint8_t>(value));
    }

    void add(float value) override final {
        image_.store(current_texel_++, static_cast<uint8_t>(value));
    }

  private:
    Byte1& image_;

    int32_t current_texel_ = 0;
};

class Float1_handler : public Json_handler::Image_handler {
  public:
    Float1_handler(Image& image) : image_(static_cast<Float1&>(image)) {}

    void add(uint32_t value) override final {
        image_.store(current_texel_++, static_cast<float>(value));
    }

    void add(float value) override final {
        image_.store(current_texel_++, value);
    }

  private:
    Float1& image_;

    int32_t current_texel_ = 0;
};

Json_handler::Image_handler::~Image_handler() {}

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
    // handle_vertex(static_cast<float>(i));

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
    image_handler_->add(static_cast<float>(d));

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
            Image::Description description(Image::Type::Byte1, dimensions_);

            image_ = new Byte1(description);

            image_handler_ = new Byte1_handler(*image_);
        } else if (Image::Type::Float1 == type_) {
            Image::Description description(Image::Type::Float1, dimensions_);

            image_ = new Float1(description);

            image_handler_ = new Float1_handler(*image_);
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

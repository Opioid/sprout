#ifndef SU_CORE_IMAGE_ENCODING_JSON_HANDLER_HPP
#define SU_CORE_IMAGE_ENCODING_JSON_HANDLER_HPP

#include <cstdint>
#include "image/image.hpp"
#include "rapidjson/reader.h"

namespace image::encoding::json {

class Image_handler {
  public:
    virtual ~Image_handler();

    virtual void add(uint32_t value) = 0;
    virtual void add(float value)    = 0;
};

class Byte1_handler : public Image_handler {
  public:
    Byte1_handler(Byte1& image);

    void add(uint32_t value) override final;

    void add(float value) override final;

  private:
    Byte1& image_;

    int32_t current_texel_ = 0;
};

class Float1_handler : public Image_handler {
  public:
    Float1_handler(Float1& image);

    void add(uint32_t value) override final;

    void add(float value) override final;

  private:
    Float1& image_;

    int32_t current_texel_ = 0;
};

class Json_handler {
  public:
    Json_handler();

    ~Json_handler();

    void clear();

    Image* image() const;

    bool Null();
    bool Bool(bool b);
    bool Int(int i);
    bool Uint(unsigned i);
    bool Int64(int64_t i);
    bool Uint64(uint64_t i);
    bool Double(double d);
    bool RawNumber(char const* str, rapidjson::SizeType length, bool copy);
    bool String(char const* str, rapidjson::SizeType length, bool copy);
    bool StartObject();
    bool Key(char const* str, rapidjson::SizeType length, bool copy);
    bool EndObject(size_t memberCount);
    bool StartArray();
    bool EndArray(size_t elementCount);

  private:
    enum class Number { Undefined, Dimensions, Data, Ignore };

    enum class String_type { Undefined, Type };

    enum class Object { Undefined, Image, Description };

    uint32_t object_level_;
    Object   top_object_;

    Number      expected_number_;
    String_type expected_string_;
    Object      expected_object_;

    Image::Type type_;

    int3 dimensions_;

    uint32_t dimensions_component_;

    uint32_t current_texel_;

    Image* image_;

    Image_handler* image_handler_;
};

}  // namespace image::encoding::json

#endif

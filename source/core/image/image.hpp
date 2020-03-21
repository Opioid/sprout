#ifndef SU_CORE_IMAGE_IMAGE_HPP
#define SU_CORE_IMAGE_IMAGE_HPP

#include "typed_image.hpp"

namespace image {

class alignas(64) Image {
  public:
    enum class Type {
        Byte1,
        Byte2,
        Byte3,
        Byte4,
        Short3,
        Short4,
        Float1,
        Float1_sparse,
        Float2,
        Float3,
        Float4
    };

    static char const* identifier();

    Image(Byte1&& image) noexcept;
    Image(Byte2&& image) noexcept;
    Image(Byte3&& image) noexcept;
    Image(Byte4&& image) noexcept;
    Image(Short3&& image) noexcept;
    Image(Short4&& image) noexcept;
    Image(Float1&& image) noexcept;
    Image(Float1_sparse&& image) noexcept;
    Image(Float2&& image) noexcept;
    Image(Float3&& image) noexcept;
    Image(Float4&& image) noexcept;

    ~Image();

    Type type() const;

    Description const& description() const;

    char* data() const;

    Byte1 const&         byte1() const;
    Byte2 const&         byte2() const;
    Byte3 const&         byte3() const;
    Byte4 const&         byte4() const;
    Short3 const&        short3() const;
    Short4 const&        short4() const;
    Float1 const&        float1() const;
    Float1_sparse const& float1_sparse() const;
    Float2 const&        float2() const;
    Float3 const&        float3() const;
    Float4 const&        float4() const;

    Byte1&         byte1();
    Byte2&         byte2();
    Byte3&         byte3();
    Byte4&         byte4();
    Short3&        short3();
    Short4&        short4();
    Float1&        float1();
    Float1_sparse& float1_sparse();
    Float2&        float2();
    Float3&        float3();
    Float4&        float4();

    size_t num_bytes() const;

  private:
    Type const type_;

    union {
        Byte1         byte1_;
        Byte2         byte2_;
        Byte3         byte3_;
        Byte4         byte4_;
        Short3        short3_;
        Short4        short4_;
        Float1        float1_;
        Float1_sparse float1_sparse_;
        Float2        float2_;
        Float3        float3_;
        Float4        float4_;
    };
};

}  // namespace image

#endif

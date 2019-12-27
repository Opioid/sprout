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
        Float1,
        Float1_sparse,
        Float2,
        Float3,
        Float4
    };

    static char const* identifier() noexcept;

    Image(Byte1&& image) noexcept;
    Image(Byte2&& image) noexcept;
    Image(Byte3&& image) noexcept;
    Image(Byte4&& image) noexcept;
    Image(Short3&& image) noexcept;
    Image(Float1&& image) noexcept;
    Image(Float1_sparse&& image) noexcept;
    Image(Float2&& image) noexcept;
    Image(Float3&& image) noexcept;
    Image(Float4&& image) noexcept;

    ~Image();

    Type type() const noexcept;

    Description const& description() const noexcept;

    char* data() const noexcept;

    Byte1 const&         byte1() const noexcept;
    Byte2 const&         byte2() const noexcept;
    Byte3 const&         byte3() const noexcept;
    Byte4 const&         byte4() const noexcept;
    Short3 const&        short3() const noexcept;
    Float1 const&        float1() const noexcept;
    Float1_sparse const& float1_sparse() const noexcept;
    Float2 const&        float2() const noexcept;
    Float3 const&        float3() const noexcept;
    Float4 const&        float4() const noexcept;

    Byte1&         byte1() noexcept;
    Byte2&         byte2() noexcept;
    Byte3&         byte3() noexcept;
    Byte4&         byte4() noexcept;
    Short3&        short3() noexcept;
    Float1&        float1() noexcept;
    Float1_sparse& float1_sparse() noexcept;
    Float2&        float2() noexcept;
    Float3&        float3() noexcept;
    Float4&        float4() noexcept;

    size_t num_bytes() const noexcept;

  private:
    Type const type_;

    union {
        Byte1         byte1_;
        Byte2         byte2_;
        Byte3         byte3_;
        Byte4         byte4_;
        Short3        short3_;
        Float1        float1_;
        Float1_sparse float1_sparse_;
        Float2        float2_;
        Float3        float3_;
        Float4        float4_;
    };
};

}  // namespace image

#endif

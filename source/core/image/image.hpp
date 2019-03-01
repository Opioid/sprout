#ifndef SU_CORE_IMAGE_IMAGE_HPP
#define SU_CORE_IMAGE_IMAGE_HPP

#include <cstddef>
#include "base/math/vector3.hpp"

namespace image {

class Image {
  public:
    enum class Type {
        Undefined,
        Byte1,
        Byte2,
        Byte3,
        Byte4,
        Float1,
        Float1_sparse,
        Float2,
        Float3,
        Float4
    };

    struct Description {
        Description(Type type) noexcept;
        Description(Type type, int2 dimensions, int32_t num_elements = 1) noexcept;
        Description(Type type, int3 const& dimensions, int32_t num_elements = 1) noexcept;

        uint64_t num_pixels() const noexcept;

        int32_t num_channels() const noexcept;

        Type type;

        int3 dimensions;

        int32_t num_elements;
    };

    Image(Description const& description) noexcept;

    virtual ~Image() noexcept;

    Description const& description() const noexcept;

    int2 dimensions2() const noexcept;

    int32_t area() const noexcept;
    int32_t volume() const noexcept;

    int2 coordinates_2(int32_t index) const noexcept;

    int3 coordinates_3(int64_t index) const noexcept;

    virtual size_t num_bytes() const noexcept = 0;

  protected:
    void resize(int3 const& dimensions, int32_t num_elements = 1) noexcept;

    Description description_;
};

}  // namespace image

#endif

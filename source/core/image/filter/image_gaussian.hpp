#ifndef SU_CORE_IMAGE_FILTER_GAUSSIAN_HPP
#define SU_CORE_IMAGE_FILTER_GAUSSIAN_HPP

#include <vector>
#include "image/typed_image.hpp"

namespace thread {
class Pool;
}

namespace image::filter {

template <typename T>
class Gaussian {
  public:
    Gaussian(float radius, float alpha) noexcept;

    void apply(Typed_image<T>& target, thread::Pool& pool) noexcept;

  private:
    Typed_image<T> scratch_;

    struct K {
        int32_t o;
        float   w;
    };

    std::vector<K> kernel_;
};

}  // namespace image::filter

#endif

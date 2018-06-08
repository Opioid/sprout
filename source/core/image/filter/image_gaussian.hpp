#pragma once

#include <vector>
#include "image/typed_image.hpp"

namespace thread {
class Pool;
}

namespace image {
namespace filter {

template <typename T>
class Gaussian {
  public:
    Gaussian(float radius, float alpha);

    void apply(Typed_image<T>& target, thread::Pool& pool);

  private:
    Typed_image<T> scratch_;

    struct K {
        int32_t o;
        float   w;
    };

    std::vector<K> kernel_;
};

}  // namespace filter
}  // namespace image

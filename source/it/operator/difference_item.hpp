#ifndef SU_IT_OPERATOR_DIFFERENCE_ITEM_HPP
#define SU_IT_OPERATOR_DIFFERENCE_ITEM_HPP

#include <string>
#include "base/math/vector.hpp"

namespace image::texture {
class Texture;
}

namespace thread {
class Pool;
}

struct Item;

namespace op {

struct Scratch {
    float max_val;
    float max_dif;
    float dif_sum;
};

class Difference_item {
  public:
    using Texture = image::texture::Texture;

    Difference_item(Item const& item);

    ~Difference_item();

    std::string name() const;

    float const* difference() const;

    float max_dif() const;

    float rmse() const;

    float psnr() const;

    void calculate_difference(Texture const* other, Scratch* scratch, float clamp, float2 clip,
                              thread::Pool& threads);

  private:
    std::string name_;

    Texture const* image_;

    float* difference_;

    float max_dif_;

    float psnr_;
    float rmse_;
};

}  // namespace op

#endif

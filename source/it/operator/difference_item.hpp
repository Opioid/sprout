#ifndef SU_IT_OPERATOR_DIFFERENCE_ITEM_HPP
#define SU_IT_OPERATOR_DIFFERENCE_ITEM_HPP

#include "base/math/vector.hpp"
#include "core/image/texture/texture.hpp"

#include <string>

namespace scene {
class Scene;
}

namespace thread {
class Pool;
}

using Threads = thread::Pool;

struct Item;

namespace op {

struct Scratch {
    float max_val;
    float max_dif;
    float dif_sum;
};

class Difference_item {
  public:
    using Scene   = scene::Scene;
    using Texture = image::texture::Texture;

    Difference_item(Item const& item, Scene const& scene);

    ~Difference_item();

    std::string name() const;

    float const* difference() const;

    float max_dif() const;

    float rmse() const;

    float psnr() const;

    void calculate_difference(Texture const& other, Scratch* scratch, float clamp, float2 clip,
                              Scene const& scene, Threads& threads);

  private:
    std::string name_;

    Texture image_;

    float* difference_;

    float max_dif_;

    float psnr_;
    float rmse_;
};

}  // namespace op

#endif

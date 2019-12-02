#ifndef SU_CORE_BAKING_WORKER_HPP
#define SU_CORE_BAKING_WORKER_HPP

#include "rendering/rendering_worker.hpp"
#include "scene/prop/interface_stack.hpp"

namespace baking {

struct Item;

class Baking_worker : public rendering::Worker {
  public:
    void baking_init(Item* items, int32_t slize_width = 0) noexcept;

    void bake(int32_t begin, int32_t end) noexcept;

  private:
    int2 coordinates(int32_t index) const noexcept;

    Item* items_;

    int32_t slice_width_;
};

}  // namespace baking

#endif

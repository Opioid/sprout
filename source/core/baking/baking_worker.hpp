#ifndef SU_CORE_BAKING_WORKER_HPP
#define SU_CORE_BAKING_WORKER_HPP

#include "rendering/rendering_worker.hpp"
#include "scene/prop/interface_stack.hpp"

namespace baking {

struct Item;

class Baking_worker : public rendering::Worker {
  public:
    void baking_init(Item* items) noexcept;

    void bake(int32_t begin, int32_t end) noexcept;

  private:
    Item* items_;
};

}  // namespace baking

#endif

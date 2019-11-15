#ifndef SU_CORE_SCENE_SHAPE_NODE_STACK_HPP
#define SU_CORE_SCENE_SHAPE_NODE_STACK_HPP

#include <cstdint>

namespace scene::shape {

class Node_stack {
  public:
    Node_stack(uint32_t size) noexcept;

    ~Node_stack() noexcept;

    bool empty() const noexcept;

    void clear() noexcept;

    void push(uint32_t value) noexcept;

    uint32_t pop() noexcept;

  private:
    [[maybe_unused]] uint32_t num_elements_;

    uint32_t end_;

    uint32_t* stack_;
};

}  // namespace scene::shape

#endif

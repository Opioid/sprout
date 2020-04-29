#ifndef SU_CORE_SCENE_SHAPE_NODE_STACK_HPP
#define SU_CORE_SCENE_SHAPE_NODE_STACK_HPP

#include <cstdint>

namespace scene::shape {

class alignas(64) Node_stack {
  public:
    Node_stack();

    ~Node_stack();

    bool empty() const;

    void clear();

    void push(uint32_t value);

    uint32_t pop();

  private:
    static uint32_t constexpr Num_elements = 127;

    uint32_t end_;

    uint32_t stack_[Num_elements];
};

}  // namespace scene::shape

#endif

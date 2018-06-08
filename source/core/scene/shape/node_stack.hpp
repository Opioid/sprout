#ifndef SU_CORE_SCENE_SHAPE_NODE_STACK_HPP
#define SU_CORE_SCENE_SHAPE_NODE_STACK_HPP

#include <cstddef>
#include <cstdint>

namespace scene::shape {

class Node_stack {
  public:
    Node_stack(uint32_t size);

    ~Node_stack();

    bool empty() const;

    void clear();

    void push(uint32_t value);

    uint32_t pop();

    size_t num_bytes() const;

  private:
    uint32_t  num_elements_;
    uint32_t  end_;
    uint32_t* stack_;
};

}  // namespace scene::shape

#endif

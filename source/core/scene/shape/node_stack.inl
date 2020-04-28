#ifndef SU_SCENE_SHAPE_NODE_STACK_INL
#define SU_SCENE_SHAPE_NODE_STACK_INL

#include "base/debug/assert.hpp"
#include "node_stack.hpp"

namespace scene::shape {

inline Node_stack::Node_stack() = default;

inline Node_stack::~Node_stack() = default;

inline bool Node_stack::empty() const {
    return 0 == end_;
}

inline void Node_stack::clear() {
    end_ = 0;
}

inline void Node_stack::push(uint32_t value) {
    SOFT_ASSERT(end_ < Num_elements);

    stack_[end_++] = value;
}

inline uint32_t Node_stack::pop() {
    return stack_[--end_];
}

}  // namespace scene::shape

#endif

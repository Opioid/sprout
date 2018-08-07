#ifndef SU_SCENE_SHAPE_NODE_STACK_INL
#define SU_SCENE_SHAPE_NODE_STACK_INL

#include "base/debug/assert.hpp"
#include "base/memory/align.hpp"
#include "node_stack.hpp"

namespace scene::shape {

inline Node_stack::Node_stack(uint32_t size) noexcept
    : num_elements_(size), stack_(memory::allocate_aligned<uint32_t>(size)) {}

inline Node_stack::~Node_stack() noexcept {
    memory::free_aligned(stack_);
}

inline bool Node_stack::empty() const noexcept {
    return 0 == end_;
}

inline void Node_stack::clear() noexcept {
    end_ = 0;
}

inline void Node_stack::push(uint32_t value) noexcept {
    SOFT_ASSERT(end_ < num_elements_);

    stack_[end_++] = value;
}

inline uint32_t Node_stack::pop() noexcept {
    return stack_[--end_];
}

inline size_t Node_stack::num_bytes() const noexcept {
    return sizeof(*this) + static_cast<size_t>(num_elements_) * sizeof(uint32_t);
}

}  // namespace scene::shape

#endif

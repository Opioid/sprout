#pragma once

#include "node_stack.hpp"
#include "base/memory/align.inl"

namespace scene { namespace shape {

inline Node_stack::Node_stack(uint32_t size) :
	num_elements_(size),
	stack_(memory::allocate_aligned<uint32_t>(size)) {}

inline Node_stack::~Node_stack() {
	memory::free_aligned(stack_);
}

inline bool Node_stack::empty() const {
	return 0 == end_;
}

inline void Node_stack::clear() {
	end_ = 0;
}

inline void Node_stack::push(uint32_t value) {
	stack_[end_++] = value;
}

inline uint32_t Node_stack::pop() {
	return stack_[--end_];
}

inline size_t Node_stack::num_bytes() const {
	return sizeof(this) + static_cast<size_t>(num_elements_) * sizeof(uint32_t);
}

}}

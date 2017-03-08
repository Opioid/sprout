#pragma once

#include "node_stack.hpp"

namespace scene { namespace shape {

inline Node_stack::Node_stack(size_t size) : stack_(size) {}

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
	return sizeof(this) + stack_.size() * sizeof(uint32_t);
}

}}

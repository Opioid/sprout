#pragma once

namespace scene { namespace shape {

Node_stack::Node_stack(size_t size) : stack_(size) {}

bool Node_stack::empty() const {
	return 0 == end_;
}

void Node_stack::clear() {
	end_ = 0;
}

void Node_stack::push(uint32_t value) {
	stack_[end_++] = value;
}

uint32_t Node_stack::pop() {
	return stack_[--end_];
}

}}

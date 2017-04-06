#pragma once

#include "base/memory/align.hpp"
#include <cstdint>
#include <cstddef>

namespace scene { namespace shape {

class Node_stack {

public:

	Node_stack(uint32_t size) :
		num_elements_(size),
		stack_(memory::allocate_aligned<uint32_t>(size)) {}

	~Node_stack() {
		memory::free_aligned(stack_);
	}

	bool empty() const {
		return 0 == end_;
	}

	bool zero() const {
		return 0 == stack_[end_];
	}

	uint32_t end() const {
		return end_;
	}

	void clear() {
		end_ = 0;
	}

	void push(uint32_t value) {
		stack_[end_++] = value;
	}

	uint32_t pop() {
		return stack_[--end_];
	}

	size_t num_bytes() const {
		return sizeof(this) + static_cast<size_t>(num_elements_) * sizeof(uint32_t);
	}

private:

	uint32_t num_elements_;
	uint32_t end_;
	uint32_t* stack_;
};

}}

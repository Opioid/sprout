#pragma once

#include <cstdint>
#include <cstddef>

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

	uint32_t num_elements_;
	uint32_t end_;
	uint32_t* stack_;
};

}

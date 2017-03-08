#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace scene { namespace shape {

class Node_stack {

public:

	Node_stack(size_t size);

	bool empty() const;

	void clear();

	void push(uint32_t value);

	uint32_t pop();

	size_t num_bytes() const;

private:

	size_t end_;
	std::vector<uint32_t> stack_;
};

}}

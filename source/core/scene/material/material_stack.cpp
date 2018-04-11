#include "material_stack.hpp"

namespace scene::material {

bool Stack::empty() const {
	return 0 == index_;
}

void Stack::clear() {
	index_ = 0;
}

const Material* Stack::top() const {
	if (index_ > 0) {
		return stack_[index_ - 1];
	}

	return nullptr;
}

void Stack::push(const Material* material) {
	if (index_ < Num_entries - 1) {
		stack_[index_] = material;
		++index_;
	}
}

void Stack::pop() {
	if (index_ > 0) {
		--index_;
	}
}

}

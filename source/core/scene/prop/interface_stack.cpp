#include "interface_stack.hpp"
#include "prop.hpp"
#include "prop_intersection.hpp"

namespace scene::prop {

const material::Material* Interface::material() const {
	return prop->material(part);
}

bool Interface::matches(const Intersection& intersection) const {
	return prop == intersection.prop && part == intersection.geo.part;
}

void Interface_stack::operator=(const Interface_stack& other) {
	index_ = other.index_;

	for (uint32_t i = 0, len = index_; i < len; ++i) {
		stack_[i] = other.stack_[i];
	}
}

bool Interface_stack::empty() const {
	return 0 == index_;
}

void Interface_stack::clear() {
	index_ = 0;
}

const Interface* Interface_stack::top() const {
	if (index_ > 0) {
		return &stack_[index_ - 1];
	}

	return nullptr;
}

float Interface_stack::top_ior() const {
	if (index_ > 0) {
		return stack_[index_ - 1].material()->ior();
	}

	return 1.f;
}

void Interface_stack::push(const Intersection& intersection) {
	if (index_ < Num_entries - 1) {
		stack_[index_] = {intersection.prop, intersection.geo.uv, intersection.geo.part};
		++index_;
	}
}

void Interface_stack::remove(const Intersection& intersection) {
	if (index_ > 0) {
	//	--index_;

		if (stack_[index_ - 1].matches(intersection)) {
			--index_;
		} else if (index_ > 1) {
			if (stack_[index_ - 2].matches(intersection)) {
				stack_[index_ - 2] = stack_[index_ - 1];
				--index_;
			}
		} else {
			pop();
		}


	}
}

void Interface_stack::pop() {
	if (index_ > 0) {
		--index_;
	}
}

}

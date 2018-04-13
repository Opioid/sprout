#include "interface_stack.hpp"
#include "prop.hpp"
#include "prop_intersection.hpp"

namespace scene::prop {

const material::Material* Interface::material() const {
	return prop->material(part);
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

void Interface_stack::push(const Intersection& intersection) {
	if (index_ < Num_entries - 1) {
		stack_[index_] = {intersection.prop, intersection.geo.part};
		++index_;
	}
}

void Interface_stack::pop() {
	if (index_ > 0) {
		--index_;
	}
}

}

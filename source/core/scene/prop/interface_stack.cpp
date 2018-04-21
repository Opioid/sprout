#include "interface_stack.hpp"
#include "prop.hpp"
#include "prop_intersection.hpp"
#include "base/memory/align.hpp"

#include "base/debug/assert.hpp"

namespace scene::prop {

const material::Material* Interface::material() const {
	return prop->material(part);
}

bool Interface::matches(const Intersection& intersection) const {
	return prop == intersection.prop && part == intersection.geo.part;
}

Interface_stack::Interface_stack() : stack_(memory::allocate_aligned<Interface>(Num_entries)) {}

Interface_stack::~Interface_stack() {
	memory::free_aligned(stack_);
}

void Interface_stack::operator=(const Interface_stack& other) {
	index_ = other.index_;

	for (uint32_t i = 0, len = index_; i < len; ++i) {
		stack_[i] = other.stack_[i];
	}
}

void Interface_stack::swap(Interface_stack& other) {
	Interface* temp = stack_;
	stack_ = other.stack_;
	other.stack_ = temp;

	index_ = other.index_;
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
	const int32_t back = index_ - 1;
	for (int32_t i = back; i >= 0; --i) {
		if (stack_[i].matches(intersection)) {
			for (int32_t j = i; j < back; ++j) {
				stack_[j] = stack_[j + 1];
			}

			--index_;
			return;
		}
	}
}

void Interface_stack::pop() {
	if (index_ > 0) {
		--index_;
	}
}

}

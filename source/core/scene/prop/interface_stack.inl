#ifndef SU_CORE_SCENE_PROP_INTERFACE_STACK_INL
#define SU_CORE_SCENE_PROP_INTERFACE_STACK_INL

#include "interface_stack.hpp"
#include "prop.hpp"
#include "prop_intersection.hpp"
#include "base/memory/align.hpp"

#include "base/debug/assert.hpp"

namespace scene::prop {

inline material::Material const* Interface_stack::Interface::material() const {
	return prop->material(part);
}

inline bool Interface_stack::Interface::matches(Intersection const& intersection) const {
	return prop == intersection.prop && part == intersection.geo.part;
}

inline Interface_stack::Interface_stack() : stack_(memory::allocate_aligned<Interface>(Num_entries)) {}

inline Interface_stack::~Interface_stack() {
	memory::free_aligned(stack_);
}

inline void Interface_stack::operator=(Interface_stack const& other) {
	index_ = other.index_;

	for (int32_t i = 0, len = index_; i < len; ++i) {
		stack_[i] = other.stack_[i];
	}
}

inline void Interface_stack::swap(Interface_stack& other) {
	Interface* temp = stack_;
	stack_ = other.stack_;
	other.stack_ = temp;

	index_ = other.index_;
}

inline bool Interface_stack::empty() const {
	return 0 == index_;
}

inline void Interface_stack::clear() {
	index_ = 0;
}

inline Interface_stack::Interface const* Interface_stack::top() const {
	if (index_ > 0) {
		return &stack_[index_ - 1];
	}

	return nullptr;
}

inline float Interface_stack::top_ior() const {
	if (index_ > 0) {
		return stack_[index_ - 1].material()->ior();
	}

	return 1.f;
}

inline void Interface_stack::push(Intersection const& intersection) {
	if (index_ < Num_entries - 1) {
		stack_[index_] = {intersection.prop, intersection.geo.uv, intersection.geo.part};
		++index_;
	}
}

inline bool Interface_stack::remove(Intersection const& intersection) {
	int32_t const back = index_ - 1;
	for (int32_t i = back; i >= 0; --i) {
		if (stack_[i].matches(intersection)) {
			for (int32_t j = i; j < back; ++j) {
				stack_[j] = stack_[j + 1];
			}

			--index_;
			return true;
		}
	}

	return false;
}

inline void Interface_stack::pop() {
	if (index_ > 0) {
		--index_;
	}
}

}

#endif

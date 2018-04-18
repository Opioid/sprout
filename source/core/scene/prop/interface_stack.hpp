#ifndef SU_CORE_SCENE_PROP_INTERFACE_STACK_HPP
#define SU_CORE_SCENE_PROP_INTERFACE_STACK_HPP

#include "base/math/vector2.hpp"
#include <cstdint>

namespace scene {

namespace material { class Material; }

namespace prop {

struct Intersection;
class Prop;

struct Interface {
	const material::Material* material() const;

	bool matches(const Intersection& intersection) const;

	const Prop* prop;
	float2		uv;
	uint32_t	part;
};

class Interface_stack {

public:

	void operator=(const Interface_stack& other);

	bool empty() const;

	void clear();

	const Interface* top() const;

	float top_ior() const;

	void push(const Intersection& intersection);
	void remove(const Intersection& intersection);
	void pop();

private:

	static constexpr uint32_t Num_entries = 16;

	uint32_t index_;
	Interface stack_[Num_entries];
};

}}

#endif

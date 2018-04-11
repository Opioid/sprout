#ifndef SU_CORE_SCENE_MATERIAL_STACK_HPP
#define SU_CORE_SCENE_MATERIAL_STACK_HPP

#include <cstdint>

namespace scene::material {

class Material;

class Stack {

public:

	bool empty() const;

	void clear();

	const Material* top() const;

	void push(const Material* material);
	void pop();

private:

	static constexpr uint32_t Num_entries = 16;

	uint32_t index_;
	const Material* stack_[Num_entries];
};

}

#endif

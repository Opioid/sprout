#pragma once

#include "base/math/vector.hpp"
#include "base/math/plane.hpp"
#include <cstdint>
#include <vector>

namespace scene {

class Prop;

namespace bvh {

class Split_candidate {
public:

	Split_candidate(const math::plane& plane, uint8_t axis);
	Split_candidate(uint8_t split_axis, float3_p p, const std::vector<Prop*>& props);

	using index = std::vector<Prop*>::iterator;

	Split_candidate(uint8_t split_axis, float3_p p, index begin, index end);

	uint64_t key() const;

	const math::plane& plane() const;

	uint8_t axis() const;

private:

	uint64_t key_;

	math::plane plane_;

	uint8_t axis_;
};

}}

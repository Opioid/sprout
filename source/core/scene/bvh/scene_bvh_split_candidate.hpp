#pragma once

#include "base/math/plane.hpp"
#include <cstdint>
#include <vector>

namespace scene {

class Prop;

namespace bvh {

class Split_candidate {

public:

	Split_candidate(const math::Plane& plane, uint8_t axis);
	Split_candidate(uint8_t split_axis, const float3& pos, const std::vector<Prop*>& props);

	using index = std::vector<Prop*>::iterator;

	Split_candidate(uint8_t split_axis, const float3& pos, index begin, index end);

	uint64_t key() const;

	const math::Plane& plane() const;

	uint8_t axis() const;

private:

	math::Plane plane_;

	uint64_t key_;

	uint8_t axis_;
};

}}

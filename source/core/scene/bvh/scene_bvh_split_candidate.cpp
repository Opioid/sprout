#include "scene_bvh_split_candidate.hpp"
#include "scene/prop/prop.hpp"
#include "base/math/bounding/aabb.inl"
#include "base/math/plane.inl"

namespace scene { namespace bvh {

Split_candidate::Split_candidate(const math::plane& plane, uint8_t axis) : plane_(plane), axis_(axis) {}

Split_candidate::Split_candidate(uint8_t split_axis, const math::float3& p, const std::vector<Prop*>& props) :
	axis_(split_axis) {
	key_ = 0;

	math::float3 n;

	switch (split_axis) {
	default:
	case 0: n = math::float3(1.f, 0.f, 0.f); break;
	case 1: n = math::float3(0.f, 1.f, 0.f); break;
	case 2: n = math::float3(0.f, 0.f, 1.f); break;
	}

	plane_ = math::create_plane(n, p);

	int num_side_0 = 0;
	int num_side_1 = 0;
	uint32_t split = 0;

	for (auto p : props) {
		bool mib = math::behind(plane_, p->aabb().min());
		bool mab = math::behind(plane_, p->aabb().max());

		if (mib && mab) {
			++num_side_0;
		} else {
			if (mib != mab) {
				++split;
			}

			++num_side_1;
		}
	}

	key_ += split;

	if (0 == num_side_0) {
		key_ += 0x1000000000000000;
	}
}

Split_candidate::Split_candidate(uint8_t split_axis, const math::float3& p, index begin, index end) :
	axis_(split_axis) {
	key_ = 0;

	math::float3 n;

	switch (split_axis) {
	default:
	case 0: n = math::float3(1.f, 0.f, 0.f); break;
	case 1: n = math::float3(0.f, 1.f, 0.f); break;
	case 2: n = math::float3(0.f, 0.f, 1.f); break;
	}

	plane_ = math::create_plane(n, p);

	int num_side_0 = 0;
	int num_side_1 = 0;
	uint32_t split = 0;

	for (index i = begin; i != end; ++i) {
		bool mib = math::behind(plane_, (*i)->aabb().min());
		bool mab = math::behind(plane_, (*i)->aabb().max());

		if (mib && mab) {
			++num_side_0;
		} else {
			if (mib != mab) {
				++split;
			}

			++num_side_1;
		}
	}

	key_ += split;

	if (0 == num_side_0) {
		key_ += 0x1000000000000000;
	}
}

uint64_t Split_candidate::key() const {
	return key_;
}

const math::plane& Split_candidate::plane() const {
	return plane_;
}

uint8_t Split_candidate::axis() const {
	return axis_;
}

}}

#ifndef SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_HPP
#define SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_HPP

#include "base/math/vector3.hpp"
#include "base/math/matrix3x3.hpp"
#include "base/math/matrix4x4.hpp"

namespace math { struct Transformation; }

namespace scene::entity {

struct Composed_transformation {
	void set(math::Transformation const& t);

	float4x4 world_to_object;
	float4x4 object_to_world;
	float3x3 rotation;
	float3   position;
	float3   scale;
};

}

#endif

#pragma once

#include "composed_transformation.hpp"
#include "base/math/quaternion.hpp"

namespace scene {

class Entity {
public:

	void transformation_at(float time, Composed_transformation& transformation) const;

	void set_transformation(const math::float3& position, const math::float3& scale, const math::quaternion& rotation);

private:

	Composed_transformation transformation_;
};

}

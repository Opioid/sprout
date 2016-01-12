#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/bounding/aabb.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace volume {

class Volume : public entity::Entity {
public:

	virtual math::float3 optical_depth(const math::Oray& ray) const = 0;

	void set_scene_aabb(const math::aabb& aabb);

protected:

	float atmosphere_y_;

private:

	virtual void on_set_transformation() final override;
};

}}

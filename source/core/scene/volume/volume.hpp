#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/bounding/aabb.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace volume {

class Volume : public entity::Entity {
public:

	virtual math::vec3 optical_depth(const math::Oray& ray) const = 0;

	virtual math::vec3 scattering() const = 0;

	virtual float phase(const math::vec3& w, const math::vec3& wp) const = 0;

	void set_scene_aabb(const math::aabb& aabb);

protected:

	math::aabb scene_bb_;

private:

	virtual void on_set_transformation() final override;
};

}}

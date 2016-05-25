#pragma once

#include "camera_stereoscopic.hpp"
#include "base/math/matrix.hpp"

namespace scene { namespace camera {

class Cubic_stereoscopic : public Stereoscopic {
public:

	enum class Layout {
		lxlmxlylmylzlmzrxrmxryrmyrzrmz,
		rxlmxryrmyrzrmzlxlmxlylmylzlmz,
	};

	Cubic_stereoscopic(Layout layout,
					   float interpupillary_distance,
					   math::int2 resolution, float ray_max_t);

	virtual uint32_t num_views() const final override;

	virtual math::int2 sensor_dimensions() const final override;

	virtual math::Recti view_bounds(uint32_t view) const final override;

	virtual void update_focus(rendering::Worker& worker) final override;

	virtual bool generate_ray(const sampler::Camera_sample& sample, uint32_t view,
							  scene::Ray& ray) const final override;

private:

	virtual void set_parameter(const std::string& name, const json::Value& value) final override;

	math::float3 left_top_;
	math::float3 d_x_;
	math::float3 d_y_;

	math::int2 sensor_dimensions_;

	math::Recti view_bounds_[12];

	math::float3x view_rotations_[12];
};

}}

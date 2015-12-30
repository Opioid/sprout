#pragma once

#include <cstdint>

namespace rendering { namespace sensor {

template<class Base, class Clamp>
class Unfiltered : public Base {
public:

	Unfiltered(math::int2 dimensions, float exposure,
			   std::unique_ptr<tonemapping::Tonemapper> tonemapper, const Clamp& clamp);

	virtual int32_t filter_radius_int() const final override;

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float4& color,
							const math::Recti& tile, const math::Recti& view_bounds) final override;

private:

	Clamp clamp_;
};

}}

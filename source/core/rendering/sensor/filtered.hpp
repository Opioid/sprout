#ifndef SU_CORE_RENDERING_SENSOR_FILTERED_HPP
#define SU_CORE_RENDERING_SENSOR_FILTERED_HPP

#include "base/math/vector4.hpp"

namespace image::texture { class Texture; }

using Texture_ptr = std::shared_ptr<image::texture::Texture>;

namespace sampler { struct Camera_sample; }

namespace rendering::sensor {

namespace filter { class Filter; }

template<class Base, class Clamp>
class Filtered : public Base {

public:

	Filtered(int2 dimensions, float exposure, const Clamp& clamp, filter::Filter const* filter);
	Filtered(int2 dimensions, float exposure, const Texture_ptr& backplate,
			 const Clamp& clamp, filter::Filter const* filter);
	~Filtered();

	virtual int32_t filter_radius_int() const override final;

	virtual int4 isolated_tile(int4 const& tile) const override final;

	virtual void add_sample(sampler::Camera_sample const& sample, float4 const&,
							int4 const& isolated_bounds, int4 const& bounds) override final;

private:

	void add_weighted_pixel(int2 pixel, float weight, float4 const& color,
							int4 const& isolated_bounds, int4 const& bounds);

	void weight_and_add_pixel(int2 pixel, float2 relative_offset, float4 const& color,
							  int4 const& isolated_bounds, int4 const& bounds);

	Clamp clamp_;

	filter::Filter const* filter_;
};

}

#endif

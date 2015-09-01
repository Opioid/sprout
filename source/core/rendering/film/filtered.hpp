#pragma once

#include "opaque.hpp"
#include <memory>

namespace rendering { namespace film {

template<class Base, class Filter>
class Filtered : public Base {
public:

	Filtered(const math::uint2& dimensions, float exposure,
			 tonemapping::Tonemapper* tonemapper, std::unique_ptr<Filter> filter);

	~Filtered();

	virtual void add_sample(const sampler::Camera_sample& sample,
							const math::float4& color, const Rectui& tile) final override;

private:

	void weight_and_add_pixel(uint32_t x, uint32_t y, math::float2 relative_offset,
							  const math::float4& color, const Rectui& tile);

	std::unique_ptr<Filter> filter_;
};

}}

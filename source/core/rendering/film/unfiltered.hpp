#pragma once

namespace rendering { namespace film {

template<class Base>
class Unfiltered : public Base {
public:

	Unfiltered(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper);

	virtual void add_sample(const sampler::Camera_sample& sample,
							const math::float4& color, const Rectui& tile) final override;
};

}}

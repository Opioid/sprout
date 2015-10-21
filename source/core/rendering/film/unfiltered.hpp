#pragma once

namespace rendering { namespace film {

template<class Base, class Clamp>
class Unfiltered : public Base {
public:

	Unfiltered(const math::uint2& dimensions, float exposure,
			   std::unique_ptr<tonemapping::Tonemapper> tonemapper, const Clamp& clamp);

	virtual void add_sample(const sampler::Camera_sample& sample,
							const math::float4& color, const Rectui& tile) final override;

private:

	Clamp clamp_;
};

}}

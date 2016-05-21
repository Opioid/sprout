#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample {

public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const = 0;

	virtual math::float3 emission() const = 0;

	virtual math::float3 attenuation() const = 0;

	virtual float ior() const = 0;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const = 0;

	virtual bool is_pure_emissive() const = 0;

	virtual bool is_transmissive() const = 0;

	virtual bool is_translucent() const = 0;

	//float absolute_n_dot_wo() const;
	float clamped_n_dot_wo() const;

	math::pfloat3 shading_normal() const;

	math::pfloat3 geometric_normal() const;

	math::float3 tangent_to_world(math::pfloat3 v) const;

	bool same_hemisphere(math::pfloat3 v) const;

	void set_basis(math::pfloat3 t, math::pfloat3 b, math::pfloat3 n,
				   math::pfloat3 geo_n, math::pfloat3 wo, bool two_sided = false);

	static math::float3 attenuation(math::pfloat3 color, float distance);

protected:

	math::float3 t_, b_, n_;
	math::float3 geo_n_;
	math::float3 wo_;
};

}}

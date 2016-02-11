#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample {
public:

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const = 0;

	virtual math::float3 emission() const = 0;

	virtual math::float3 attenuation() const = 0;

	virtual float ior() const = 0;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const = 0;

	virtual bool is_pure_emissive() const = 0;

	virtual bool is_transmissive() const = 0;

	virtual bool is_translucent() const = 0;

	//float absolute_n_dot_wo() const;
	float clamped_n_dot_wo() const;

	const math::float3& shading_normal() const;

	const math::float3& geometric_normal() const;

	math::float3 tangent_to_world(const math::float3& v) const;

	bool same_hemisphere(const math::float3& v) const;

	void set_basis(const math::float3& t, const math::float3& b, const math::float3& n,
				   const math::float3& geo_n, const math::float3& wo, bool two_sided = false);

	static math::float3 attenuation(const math::float3& color, float distance);

protected:

	math::float3 t_, b_, n_;
	math::float3 geo_n_;
	math::float3 wo_;
};

}}

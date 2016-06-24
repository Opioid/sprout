#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample {

public:

	struct Layer {
		void set_basis(float3_p t, float3_p b, float3_p n, float sign);

		float clamped_n_dot(float3_p v) const;

		float3 tangent_to_world(float3_p v) const;

		float3 t, b, n;
	};

	virtual float3_p shading_normal() const = 0;

	virtual float3 tangent_to_world(float3_p v) const = 0;

	virtual float3 evaluate(float3_p wi, float& pdf) const = 0;

	virtual float3 radiance() const = 0;

	virtual float3 attenuation() const = 0;

	virtual float ior() const = 0;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const = 0;

	virtual bool is_pure_emissive() const = 0;

	virtual bool is_transmissive() const = 0;

	virtual bool is_translucent() const = 0;

	float3_p geometric_normal() const;

	bool same_hemisphere(float3_p v) const;

	float set_basis(float3_p geo_n, float3_p wo, bool two_sided = false);

	static float3 attenuation(float3_p color, float distance);

//protected:

	float3 geo_n_;
	float3 wo_;
};

}}

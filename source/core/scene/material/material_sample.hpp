#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample {
public:

	virtual math::vec3 evaluate(math::pvec3 wi, float& pdf) const = 0;

	virtual math::vec3 emission() const = 0;

	virtual math::vec3 attenuation() const = 0;

	virtual float ior() const = 0;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const = 0;

	virtual bool is_pure_emissive() const = 0;

	virtual bool is_transmissive() const = 0;

	virtual bool is_translucent() const = 0;

	//float absolute_n_dot_wo() const;
	float clamped_n_dot_wo() const;

	const math::vec3& shading_normal() const;

	const math::vec3& geometric_normal() const;

	math::vec3 tangent_to_world(math::pvec3 v) const;

	bool same_hemisphere(math::pvec3 v) const;

	void set_basis(const math::vec3& t, const math::vec3& b, const math::vec3& n,
				   const math::vec3& geo_n, const math::vec3& wo, bool two_sided = false);

	static math::vec3 attenuation(math::pvec3 color, float distance);

protected:

	math::vec3 t_, b_, n_;
	math::vec3 geo_n_;
	math::vec3 wo_;
};

}}

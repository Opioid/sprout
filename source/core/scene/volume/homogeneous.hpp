#pragma once

#include "volume.hpp"

namespace scene { namespace volume {

class Homogeneous : public Volume {
public:

	Homogeneous(const math::vec3& absorption, const math::vec3& scattering);

	virtual math::vec3 optical_depth(const math::Oray& ray) const final override;

	virtual math::vec3 scattering() const final override;

	virtual float phase(const math::vec3& w, const math::vec3& wp) const final override;

private:

	math::vec3 absorption_;
	math::vec3 scattering_;
};

}}

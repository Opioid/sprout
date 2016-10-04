#pragma once

#include "volume.hpp"

namespace scene { namespace volume {

class Homogeneous : public Volume {

public:

	virtual float3 optical_depth(const math::Oray& ray) const final override;

	virtual float3 scattering() const final override;

	virtual float phase(float3_p w, float3_p wp) const final override;

private:


};

}}

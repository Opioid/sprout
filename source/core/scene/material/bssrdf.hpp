#ifndef SU_CORE_SCENE_MATERIAL_BSSRDF_HPP
#define SU_CORE_SCENE_MATERIAL_BSSRDF_HPP

#include "base/math/vector3.hpp"

namespace scene::material {

class BSSRDF {

public:

	float3 optical_depth(float length) const;

	const float3& scattering() const;

	void set(const float3& absorption, const float3& scattering);

private:

	float3 absorption_;
	float3 scattering_;
};

}

#endif

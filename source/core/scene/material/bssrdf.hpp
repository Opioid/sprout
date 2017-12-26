#ifndef SU_CORE_SCENE_MATERIAL_BSSRDF_HPP
#define SU_CORE_SCENE_MATERIAL_BSSRDF_HPP

#include "base/math/vector3.hpp"

namespace scene::material {

class BSSRDF {

public:

	BSSRDF() = default;

	BSSRDF(const float3& absorption_coefficient, const float3& scattering_coefficient,
		   float anisotropy);

	float3 optical_depth(float length) const;

	float3 extinction_coefficient() const;

	const float3& absorption_coefficient() const;
	const float3& scattering_coefficient() const;

	bool is_scattering() const;

	void set(const float3& absorption_coefficient, const float3& scattering_coefficient,
			 float anisotropy);

private:

	float3 absorption_coefficient_;
	float3 scattering_coefficient_;
	float  anisotropy_;
};

}

#endif

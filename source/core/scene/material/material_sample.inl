#pragma once

#include "material_sample.hpp"

namespace scene { namespace material {

template<>
void Sample::set_basis<true>(const math::float3& t, const math::float3& b, const math::float3& n,
							 const math::float3& geo_n, const math::float3& wo) {
	t_ = t;
	b_ = b;

	if (math::dot(geo_n, wo) < 0.f) {
		n_ = -n;
		geo_n_ = -geo_n;
	} else {
		n_ = n;
		geo_n_ = geo_n;
	}

	wo_ = wo;
}

template<>
void Sample::set_basis<false>(const math::float3& t, const math::float3& b, const math::float3& n,
							  const math::float3& geo_n, const math::float3& wo) {
	t_ = t;
	b_ = b;
	n_ = n;
	geo_n_ = geo_n;
	wo_ = wo;
}

}}


#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Filmic : public Tonemapper {
public:

	Filmic(const math::float3& linear_white);

	virtual math::float3 tonemap(const math::float3& color) const final override;

private:

	static math::float3 tonemap_function(const math::float3& color);

	math::float3 linear_white_;
};

}}}

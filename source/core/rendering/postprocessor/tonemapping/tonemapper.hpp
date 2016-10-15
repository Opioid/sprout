#pragma once

#include "rendering/postprocessor/postprocessor.hpp"
#include "base/math/vector.hpp"

namespace rendering { namespace postprocessor { namespace tonemapping {

class Tonemapper : public Postprocessor {

public:

	virtual ~Tonemapper();


	virtual void init(const scene::camera::Camera& camera) final override;

private:

	virtual void apply(int32_t begin, int32_t end,
					   const image::Image_float_4& source,
					   image::Image_float_4& destination) const final override;

protected:

	virtual float3 tonemap(float3_p color) const = 0;

	static float normalization_factor(float hdr_max, float tonemapped_max);
};

}}}

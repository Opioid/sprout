#pragma once

#include "rendering/postprocessor/postprocessor.hpp"
#include "base/math/vector3.hpp"

namespace rendering { namespace postprocessor { namespace tonemapping {

class Tonemapper : public Postprocessor {

public:

	virtual ~Tonemapper();


	virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) final override;

	virtual size_t num_bytes() const final override;

private:

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Float_4& source,
					   image::Float_4& destination) final override;

protected:

	virtual float3 tonemap(float3_p color) const = 0;

	static float normalization_factor(float hdr_max, float tonemapped_max);
};

}}}

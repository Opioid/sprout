#pragma once

#include "rendering/postprocessor/postprocessor.hpp"
#include "base/math/vector3.hpp"

namespace rendering { namespace postprocessor { namespace tonemapping {

class Tonemapper : public Postprocessor {

public:

	virtual ~Tonemapper();

	virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) final override;

	virtual size_t num_bytes() const final override;

protected:

	static float normalization_factor(float hdr_max, float tonemapped_max);
};

}}}

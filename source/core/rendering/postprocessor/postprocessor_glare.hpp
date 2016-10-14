#pragma once

#include "postprocessor.hpp"

namespace rendering { namespace postprocessor {

class Glare : public Postprocessor {

public:

	virtual void init(const scene::camera::Camera& camera) final override;

private:

	virtual void apply(int32_t begin, int32_t end,
					   const image::Image_float_4& source,
					   image::Image_float_4& destination) const final override;
};

}}

#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace postprocessor { namespace tonemapping {

class Identity : public Tonemapper {

private:

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Float4& source,
					   image::Float4& destination) override final;
};

}}}

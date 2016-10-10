#pragma once

#include "density.hpp"
#include "image/texture/texture_types.hpp"

namespace scene { namespace volume {

class Grid : public Density {

public:

	Grid(Texture_ptr grid);

private:

	virtual float density(float3_p p, Worker& worker,
						  Sampler_filter filter) const final override;

	virtual void set_parameter(const std::string& name,
	                           const json::Value& value) final override;

	Texture_adapter grid_;
};

}}

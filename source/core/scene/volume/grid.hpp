#ifndef SU_CORE_SCENE_VOLUME_GRID_HPP
#define SU_CORE_SCENE_VOLUME_GRID_HPP

#include "density.hpp"
#include "image/texture/texture_adapter.hpp"
#include "image/texture/texture_types.hpp"

namespace scene::volume {

class Grid : public Density {

public:

	Grid(Texture_ptr grid);

private:

	virtual float density(const float3& p, Sampler_filter filter,
						  Worker& worker) const override final;

	virtual void set_parameter(const std::string& name,
	                           const json::Value& value) override final;

	Texture_adapter grid_;
};

}

#endif

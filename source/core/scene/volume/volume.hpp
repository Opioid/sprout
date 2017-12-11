#ifndef SU_CORE_SCENE_VOLUME_VOLUME_HPP
#define SU_CORE_SCENE_VOLUME_VOLUME_HPP

#include "scene/prop/prop.hpp"
#include <string>

namespace scene::volume {

class Volume final : public prop::Prop {

public:

	Volume();

	virtual void set_parameters(const json::Value& parameters) override final;

	void set_scene_aabb(const math::AABB& aabb);

protected:

	virtual void set_parameter(const std::string& name, const json::Value& value);

	bool match_scene_scale_;
};

}

#endif

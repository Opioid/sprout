#pragma once

#include "scene/entity/entity.hpp"

namespace scene {

class Volume : public entity::Entity {
public:

private:

	virtual void on_set_transformation() final override;
};

}

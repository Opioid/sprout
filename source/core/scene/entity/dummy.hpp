#pragma once

#include "entity.hpp"

namespace scene { namespace entity {

class Dummy : public Entity {
public:

private:

	virtual void on_set_transformation() final override;
};

}}

#pragma once

#include "entity.hpp"

namespace scene { namespace entity {

class Dummy : public Entity {

public:

	virtual void set_parameters(const json::Value& parameters) override final;

private:

	virtual void on_set_transformation() override final;
};

}}

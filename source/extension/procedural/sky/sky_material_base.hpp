#pragma once

#include "core/scene/material/material.hpp"

namespace procedural { namespace sky {

class Model;

class Material : public scene::material::Material {

public:

	using Sampler_filter = scene::material::Sampler_settings::Filter;

	Material(Model& model);

protected:

	Model& model_;
};

}}

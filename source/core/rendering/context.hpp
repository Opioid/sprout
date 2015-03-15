#pragma once

#include "rendering/sampler/sampler.hpp"
#include "scene/camera/camera.hpp"
#include <memory>

namespace rendering {

struct Context {
	std::shared_ptr<camera::Camera>	  camera;
	std::shared_ptr<sampler::Sampler> sampler;
};

}

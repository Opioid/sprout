#pragma once

#include "scene/camera/camera.hpp"
#include "image/typed_image.hpp"

namespace thread { class Pool; }

namespace rendering {

namespace sensor {

class Sensor;

namespace tonemapping { class Tonemapper; }

}


namespace postprocessor {

class Pipeline {

public:

	void init(const scene::camera::Camera& camera);

	void apply(const sensor::Sensor& sensor, image::Image_float_4& target,
			   thread::Pool& pool) const;

private:

};

}}


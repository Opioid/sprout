#pragma once

#include "scene/camera/camera.hpp"
#include "image/typed_image.hpp"
#include <vector>

namespace thread { class Pool; }

namespace rendering {

namespace sensor {

class Sensor;

namespace tonemapping { class Tonemapper; }

}


namespace postprocessor {

class Postprocessor;

class Pipeline {

public:

	Pipeline();
	~Pipeline();

	void reserve(size_t num_pps);

	void add(Postprocessor* pp);

	void init(const scene::camera::Camera& camera, thread::Pool& pool);

	size_t num_bytes() const;

	void apply(const sensor::Sensor& sensor, image::Float4& target,
			   thread::Pool& pool);

private:

	image::Float4 scratch_;

	std::vector<Postprocessor*> postprocessors_;
};

}}


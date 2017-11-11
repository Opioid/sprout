#ifndef SU_CORE_RENDERING_POSTPROCESSOR_PIPELINE_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_PIPELINE_HPP

#include "scene/camera/camera.hpp"
#include "image/typed_image.hpp"
#include <vector>

namespace thread { class Pool; }

namespace rendering {

namespace sensor { class Sensor; }

namespace postprocessor {

class Postprocessor;

class Pipeline {

public:

	Pipeline();
	~Pipeline();

	void reserve(size_t num_pps);

	void add(Postprocessor* pp);

	void init(const scene::camera::Camera& camera, thread::Pool& pool);

	bool has_alpha_transparency(bool alpha_in) const;

	void apply(const sensor::Sensor& sensor, image::Float4& target, thread::Pool& pool);

	size_t num_bytes() const;

private:

	image::Float4 scratch_;

	std::vector<Postprocessor*> postprocessors_;
};

}}

#endif

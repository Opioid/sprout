#ifndef SU_CORE_RENDERING_POSTPROCESSOR_PIPELINE_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_PIPELINE_HPP

#include "scene/camera/camera.hpp"
#include "image/typed_image_fwd.hpp"
#include "image/typed_image.hpp"
#include <vector>
#include <memory>

namespace thread { class Pool; }

namespace rendering {

namespace sensor { class Sensor; }

namespace postprocessor {

class Postprocessor;

class Pipeline {

public:

	Pipeline() = default;
	~Pipeline();

	void reserve(size_t num_pps);

	void add(std::unique_ptr<Postprocessor> pp);

	void init(const scene::camera::Camera& camera, thread::Pool& pool);

	bool has_alpha_transparency(bool alpha_in) const;

	void apply(const sensor::Sensor& sensor, image::Float4& target, thread::Pool& pool);

	size_t num_bytes() const;

private:

	image::Float4 scratch_;

	std::vector<std::unique_ptr<Postprocessor>> postprocessors_;
};

}}

#endif

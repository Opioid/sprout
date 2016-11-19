#pragma once

#include "rendering/postprocessor/postprocessor_pipeline.hpp"
#include <memory>

namespace scene { namespace camera { class Camera; } }

namespace take {

struct View {
	View() {};

	void init(thread::Pool& pool) {
		if (camera) {
			pipeline.init(*camera, pool);
		}
	}

	std::shared_ptr<scene::camera::Camera> camera;

	uint32_t num_samples_per_pixel = 0;

	rendering::postprocessor::Pipeline pipeline;

	uint32_t start_frame = 0;
	uint32_t num_frames  = 1;
};

}

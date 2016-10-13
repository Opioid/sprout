#pragma once

#include "rendering/postprocessor/postprocessor_pipeline.hpp"
#include <memory>

namespace scene { namespace camera { class Camera; } }

namespace take {

struct View {
	std::shared_ptr<scene::camera::Camera> camera;

	rendering::postprocessor::Pipeline pipeline;

	uint32_t start_frame = 0;
	uint32_t num_frames  = 1;
};

}

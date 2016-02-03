#pragma once

#include "rendering_worker.hpp"

namespace rendering {

class Camera_worker : public Worker {
public:

	void render(scene::camera::Camera& camera, uint32_t view, const math::Recti& tile,
				uint32_t sample_begin, uint32_t sample_end,
				float absolute_time, float tick_slice,
				float normalized_tick_offset, float normalized_tick_slice);
};

}

#pragma once

#include "rendering_worker.hpp"

namespace scene { namespace camera { class Camera; }}

namespace rendering {

class Camera_worker : public Worker {

public:

	void render(scene::camera::Camera& camera, uint32_t view,
				const int4& tile, uint32_t sample_begin, uint32_t sample_end,
				float normalized_tick_offset, float normalized_tick_slice);
};

}

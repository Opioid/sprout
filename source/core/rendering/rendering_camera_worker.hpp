#ifndef SU_CORE_RENDERING_CAMERA_WORKER_HPP
#define SU_CORE_RENDERING_CAMERA_WORKER_HPP

#include "rendering_worker.hpp"

namespace scene::camera { class Camera; }

namespace rendering {

class alignas(64) Camera_worker : public Worker {

public:

	void render(scene::camera::Camera& camera, uint32_t view,
				const int4& tile, uint32_t sample_begin, uint32_t sample_end,
				float normalized_tick_offset, float normalized_tick_slice);

	size_t num_bytes() const;
};

}

#endif

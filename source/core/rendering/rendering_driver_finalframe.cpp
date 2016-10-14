#include "rendering_driver_finalframe.hpp"
#include "exporting/exporting_sink.hpp"
#include "logging/logging.hpp"
#include "progress/progress_sink.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/camera/camera.hpp"
#include "take/take_view.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/vector.inl"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"

namespace rendering {

Driver_finalframe::Driver_finalframe(Surface_integrator_factory surface_integrator_factory,
									 Volume_integrator_factory volume_integrator_factory,
									 std::shared_ptr<sampler::Factory> sampler_factory,
									 scene::Scene& scene,
									 take::View& view,
									 thread::Pool& thread_pool) :
	Driver(surface_integrator_factory, volume_integrator_factory,
		   sampler_factory, scene, view, thread_pool) {}

void Driver_finalframe::render(exporting::Sink& exporter, progress::Sink& progressor) {
	auto& camera = *view_.camera;
	auto& sensor = camera.sensor();

	const uint32_t num_samples_per_iteration = sampler_factory_->num_samples_per_iteration();
	const uint32_t progress_range = calculate_progress_range(scene_, camera, tiles_.size(),
															 num_samples_per_iteration);

	float tick_offset = scene_.seek(static_cast<float>(view_.start_frame) * camera.frame_duration(),
									thread_pool_);
	float tick_rest   = scene_.tick_duration() - tick_offset;

	camera.update_focus(workers_[0]);

	for (uint32_t f = 0; f < view_.num_frames; ++f) {
		uint32_t current_frame = view_.start_frame + f;
		logging::info("Frame " + string::to_string(current_frame));

		auto render_start = std::chrono::high_resolution_clock::now();

		sensor.clear();
		current_sample_ = 0;

		progressor.start(progress_range);

		if (0.f == camera.frame_duration()) {
			scene_.tick(thread_pool_);
			camera.update_focus(workers_[0]);
			render_subframe(0.f, 0.f, 1.f, progressor);
		} else if (!camera.motion_blur()) {
			float frame_offset = 0.f;
			float frame_rest = camera.frame_duration();

			bool rendered = false;

			while (frame_rest > 0.f) {
				if (tick_rest <= 0.f) {
					scene_.tick(thread_pool_);
					camera.update_focus(workers_[0]);
					tick_offset = 0.f;
					tick_rest = scene_.tick_duration();
				}

				float subframe_slice = std::min(tick_rest, frame_rest);

				if (!rendered) {
					float normalized_tick_offset = tick_offset / scene_.tick_duration();

					render_subframe(normalized_tick_offset, 0.f, 1.f,
									progressor);

					rendered = true;
				}

				tick_offset += subframe_slice;
				tick_rest   -= subframe_slice;

				frame_offset += subframe_slice;
				frame_rest   -= subframe_slice;
			}
		} else {
			float frame_offset = 0.f;
			float frame_rest = camera.frame_duration();

			while (frame_rest > 0.f) {
				if (tick_rest <= 0.f) {
					scene_.tick(thread_pool_);
					camera.update_focus(workers_[0]);
					tick_offset = 0.f;
					tick_rest = scene_.tick_duration();
				}

				float subframe_slice = std::min(tick_rest, frame_rest);

				float normalized_tick_offset = tick_offset / scene_.tick_duration();
				float normalized_tick_slice  = subframe_slice / scene_.tick_duration();
				float normalized_frame_slice = subframe_slice / camera.frame_duration();

				render_subframe(normalized_tick_offset, normalized_tick_slice,
								normalized_frame_slice, progressor);

				tick_offset += subframe_slice;
				tick_rest   -= subframe_slice;

				frame_offset += subframe_slice;
				frame_rest   -= subframe_slice;
			}
		}

		progressor.end();

		auto render_duration = chrono::seconds_since(render_start);
		logging::info("Render time " + string::to_string(render_duration) + " s");

		auto export_start = std::chrono::high_resolution_clock::now();
		view_.pipeline.apply(sensor, target_, thread_pool_);
		exporter.write(target_, current_frame, thread_pool_);
		auto export_duration = chrono::seconds_since(export_start);
		logging::info("Export time " + string::to_string(export_duration) + " s");
	}
}

void Driver_finalframe::render_subframe(float normalized_tick_offset,
										float normalized_tick_slice,
										float normalized_frame_slice,
										progress::Sink& progressor) {
	float num_samples = static_cast<float>(sampler_factory_->num_samples_per_iteration());
	float samples_per_slice = normalized_frame_slice * num_samples;

	uint32_t sample_begin = current_sample_;
	uint32_t sample_range = std::max(static_cast<uint32_t>(samples_per_slice + 0.5f), 1u);
	uint32_t sample_end   = std::min(sample_begin + sample_range,
									 sampler_factory_->num_samples_per_iteration());

	if (sample_begin == sample_end) {
		return;
	}

	for (uint32_t v = 0, len = view_.camera->num_views(); v < len; ++v) {
		tiles_.restart();

		thread_pool_.run(
			[this, v, &progressor, sample_begin, sample_end,
			 normalized_tick_offset, normalized_tick_slice]
			(uint32_t index) {
				auto& worker = workers_[index];

				for (;;) {
					math::Recti tile;
					if (!tiles_.pop(tile)) {
						break;
					}

					worker.render(*view_.camera, v, tile, sample_begin, sample_end,
								  normalized_tick_offset, normalized_tick_slice);

					progressor.tick();
				}
			}
		);
	}

	current_sample_ = sample_end;
}

uint32_t Driver_finalframe::calculate_progress_range(const scene::Scene& scene,
													 const scene::camera::Camera& camera,
													 uint32_t num_tiles,
													 uint32_t num_samples_per_iteration) {
	float num_subframes = 1.f;

	if (camera.frame_duration() > 0.f && camera.motion_blur()) {
		num_subframes = std::min(camera.frame_duration() / scene.tick_duration(),
								 static_cast<float>(num_samples_per_iteration));
	}

	float range = static_cast<float>(num_tiles * camera.num_views()) * num_subframes;

	return static_cast<uint32_t>(range);
}

}

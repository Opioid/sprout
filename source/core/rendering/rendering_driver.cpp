#include "rendering_driver.hpp"
#include "rendering_context.hpp"
#include "rendering_worker.hpp"
#include "exporting/exporting_sink.hpp"
#include "logging/logging.hpp"
#include "rendering/sensor/sensor.hpp"
#include "rendering/integrator/integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "progress/progress_sink.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"

namespace rendering {

class Tile_queue {
public:

	Tile_queue(size_t num_tiles) : tiles_(num_tiles), current_produce_(0), current_consume_(0) {}

	size_t size() {
		return tiles_.size();
	}

	void restart() {
		current_produce_ = tiles_.size();
		current_consume_ = 0;
	}

	bool pop(Rectui& tile) {
		std::lock_guard<std::mutex> lock(mutex_);

		if (current_consume_ < tiles_.size()) {
			tile = tiles_[current_consume_++];
			return true;
		}

		return false;
	}

	void push(const Rectui& tile) {
		tiles_[current_produce_++] = tile;
	}

private:

	std::vector<Rectui> tiles_;
	size_t current_produce_;
	size_t current_consume_;

	std::mutex mutex_;
};

Driver::Driver(std::shared_ptr<Surface_integrator_factory> surface_integrator_factory,
				   std::shared_ptr<sampler::Sampler> sampler) :
	surface_integrator_factory_(surface_integrator_factory),
	sampler_(sampler),
	tile_dimensions_(math::uint2(32, 32)),
	current_pixel_(math::uint2(0, 0)) {}

void Driver::render(scene::Scene& scene, const Context& context, thread::Pool& pool,
					exporting::Sink& exporter, progress::Sink& progressor) {
	auto& camera = *context.camera;
	auto& sensor   = camera.sensor();

	auto dimensions = sensor.dimensions();

	size_t num_tiles = static_cast<size_t>(std::ceil(static_cast<float>(dimensions.x)
												   / static_cast<float>(tile_dimensions_.x)))
					 * static_cast<size_t>(std::ceil(static_cast<float>(dimensions.y)
												   / static_cast<float>(tile_dimensions_.y)));

	Tile_queue tiles(num_tiles);

	for (;;) {
		tiles.push(Rectui{current_pixel_, math::min(current_pixel_ + tile_dimensions_, dimensions)});

		if (!advance_current_pixel(dimensions)) {
			break;
		}
	}

	uint32_t num_workers = pool.num_threads();
	std::vector<Camera_worker> workers(num_workers);
	for (uint32_t i = 0; i < num_workers; ++i) {
		math::random::Generator rng(i + 0, i + 1, i + 2, i + 3);
		workers[i].init(i, rng, *surface_integrator_factory_, *sampler_, scene);
	}

	std::chrono::high_resolution_clock clock;
	const size_t progress_range = calculate_progress_range(scene, camera, tiles.size());

	float tick_offset = scene.seek(static_cast<float>(context.start_frame) * camera.frame_duration(), pool);
	float tick_rest   = scene.tick_duration() - tick_offset;

	for (uint32_t f = 0; f < context.num_frames; ++f) {
		uint32_t current_frame = context.start_frame + f;
		logging::info("Frame " + string::to_string(current_frame));

		auto render_start = clock.now();

		sensor.clear();
		current_sample_ = 0;

		progressor.start(progress_range);

		if (0.f == camera.frame_duration()) {
			scene.tick(pool);
			camera.update_focus(workers[0]);
			render_subframe(camera, 0.f, 0.f, 1.f, tiles, workers, pool, progressor);
		} else if (!camera.motion_blur()) {
			float frame_offset = 0.f;
			float frame_rest = camera.frame_duration();

			bool rendered = false;

			while (frame_rest > 0.f) {
				if (tick_rest <= 0.f) {
					scene.tick(pool);
					camera.update_focus(workers[0]);
					tick_offset = 0.f;
					tick_rest = scene.tick_duration();
				}

				float subframe_slice = std::min(tick_rest, frame_rest);

				if (!rendered) {
					float normalized_tick_offset = tick_offset / scene.tick_duration();

					render_subframe(camera,
									normalized_tick_offset, 0.f, 1.f,
									tiles, workers, pool, progressor);

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
					scene.tick(pool);
					camera.update_focus(workers[0]);
					tick_offset = 0.f;
					tick_rest = scene.tick_duration();
				}

				float subframe_slice = std::min(tick_rest, frame_rest);

				float normalized_tick_offset = tick_offset / scene.tick_duration();
				float normalized_tick_slice  = subframe_slice / scene.tick_duration();
				float normalized_frame_slice = subframe_slice / camera.frame_duration();

				render_subframe(camera,
								normalized_tick_offset, normalized_tick_slice, normalized_frame_slice,
								tiles, workers, pool, progressor);

				tick_offset += subframe_slice;
				tick_rest   -= subframe_slice;

				frame_offset += subframe_slice;
				frame_rest   -= subframe_slice;
			}
		}

		progressor.end();

		auto render_duration = chrono::duration_to_seconds(clock.now() - render_start);
		logging::info("Render time " + string::to_string(render_duration) + " s");

		auto export_start = clock.now();
		exporter.write(sensor.resolve(pool), current_frame, pool);
		auto export_duration = chrono::duration_to_seconds(clock.now() - export_start);
		logging::info("Export time " + string::to_string(export_duration) + " s");
	}
}

void Driver::render_subframe(scene::camera::Camera& camera,
							 float normalized_tick_offset, float normalized_tick_slice, float normalized_frame_slice,
							 Tile_queue& tiles, std::vector<Camera_worker>& workers, thread::Pool& pool,
							 progress::Sink& progressor) {
	tiles.restart();

	float num_samples = static_cast<float>(sampler_->num_samples_per_iteration());

	uint32_t sample_begin = current_sample_;
	uint32_t sample_range = std::max(static_cast<uint32_t>(normalized_frame_slice * num_samples + 0.5f), 1u);
	uint32_t sample_end   = std::min(sample_begin + sample_range, sampler_->num_samples_per_iteration());

	if (sample_begin == sample_end) {
		return;
	}

	pool.run(
		[&workers, &camera, &tiles, &progressor,
		 sample_begin, sample_end, normalized_tick_offset, normalized_tick_slice](uint32_t index) {
			auto& worker = workers[index];

			for (;;) {
				Rectui tile;
				if (!tiles.pop(tile)) {
					break;
				}

				worker.render(camera, tile, sample_begin, sample_end, normalized_tick_offset, normalized_tick_slice);
				progressor.tick();
			}
		}
	);

	current_sample_ = sample_end;
}

bool Driver::advance_current_pixel(math::uint2 dimensions) {
	current_pixel_.x += tile_dimensions_.x;

	if (current_pixel_.x >= dimensions.x) {
		current_pixel_.x = 0;
		current_pixel_.y += tile_dimensions_.y;
	}

	if (current_pixel_.y >= dimensions.y) {
		return false;
	}

	return true;
}

size_t Driver::calculate_progress_range(const scene::Scene& scene, const scene::camera::Camera& camera,
										size_t num_tiles) const {
	const float num_subframes = 0.f == camera.frame_duration() || !camera.motion_blur()
							  ? 1.f : std::min(camera.frame_duration() / scene.tick_duration(),
											   static_cast<float>(sampler_->num_samples_per_iteration()));

	return static_cast<size_t>(static_cast<float>(num_tiles) * num_subframes);
}

}

#include "renderer.hpp"
#include "context.hpp"
#include "worker.hpp"
#include "rendering/film/film.hpp"
#include "rendering/integrator/integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "progress/sink.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include <thread>
#include <mutex>

namespace rendering {

class Tile_queue {
public:

	Tile_queue(size_t num_tiles) : tiles_(num_tiles), current_produce_(0), current_consume_(0) {}

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

Renderer::Renderer(std::shared_ptr<Surface_integrator_factory> surface_integrator_factory,
				   std::shared_ptr<sampler::Sampler> sampler) :
	surface_integrator_factory_(surface_integrator_factory), sampler_(sampler),
	tile_dimensions_(math::uint2(32, 32)), current_pixel_(math::uint2(0, 0)) {}

void Renderer::render(const scene::Scene& scene, const Context& context, uint32_t num_workers, progress::Sink& progressor) {
	auto& film = context.camera->film();

	auto& dimensions = film.dimensions();

	size_t num_tiles = static_cast<size_t>(std::ceil(static_cast<float>(dimensions.x) / static_cast<float>(tile_dimensions_.x)))
					 * static_cast<size_t>(std::ceil(static_cast<float>(dimensions.y) / static_cast<float>(tile_dimensions_.y)));

	Tile_queue tiles(num_tiles);

	for (;;) {
		tiles.push(Rectui{current_pixel_, math::min(current_pixel_ + tile_dimensions_, dimensions)});

		if (!advance_current_pixel(dimensions)) {
			break;
		}
	}

	progressor.start(num_tiles);

	std::vector<Worker> workers(num_workers);
	for (size_t i = 0; i < num_workers; ++i) {
		math::random::Generator rng(i + 0, i + 1, i + 2, i + 3);
		workers[i].init(i, rng, *surface_integrator_factory_, *sampler_, scene);
	}

	uint32_t sample_start = 0;
	uint32_t sample_end = sampler_->num_samples_per_iteration();

	std::vector<std::thread> threads;

	for (size_t i = 0; i < num_workers; ++i) {
		threads.push_back(std::thread(
			[&workers, &context, &tiles, &progressor, sample_start, sample_end](size_t index) {
				auto& worker = workers[index];

				for (;;) {
					Rectui tile;
					if (!tiles.pop(tile)) {
						break;
					}

					worker.render(*context.camera, tile, sample_start, sample_end);
					progressor.tick();
				}
			},
		i));
	}

	for (size_t i = 0, len = threads.size(); i < len; ++i) {
		threads[i].join();
	}

	progressor.end();
}

bool Renderer::advance_current_pixel(const math::uint2& dimensions) {
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

}

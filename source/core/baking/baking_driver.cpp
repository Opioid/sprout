#include "baking_driver.hpp"
#include "baking_worker.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"
#include "image/encoding/png/png_writer.hpp"
#include "image/typed_image.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"

#include <fstream>

namespace baking {

Driver::Driver(take::Take& take, Scene& scene, thread::Pool& thread_pool,
               uint32_t max_material_sample_size, progress::Sink& progressor) noexcept
    : progressor_(progressor) {}

Driver::~Driver() noexcept {}

void Driver::render() noexcept {}

}  // namespace baking

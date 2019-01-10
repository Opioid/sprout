#ifndef SU_EXTENSION_PROCEDURAL_FLUID_SIMULATION_HPP
#define SU_EXTENSION_PROCEDURAL_FLUID_SIMULATION_HPP

#include "base/math/aabb.hpp"
#include "core/scene/scene_constants.hpp"
#include "fluid_grid.hpp"

namespace thread {
class Pool;
}

namespace procedural::fluid {

struct Particle;
struct Vorton;

class Simulation {
  public:
    Simulation(int3 const& dimensions, int3 const& visualization_dimensions) noexcept;

    ~Simulation() noexcept;

    void set_aabb(AABB const& aabb) noexcept;

    void simulate(thread::Pool& pool) noexcept;

    Vorton* vortons() noexcept;

    uint32_t num_vortons() const noexcept;

    Particle* tracers() noexcept;

    uint32_t num_tracers() const noexcept;

    float3 world_to_texture_point(float3 const& v) const noexcept;

  private:
    void compute_velocity_grid(thread::Pool& pool) noexcept;

    void stretch_and_tilt_vortons(thread::Pool& pool) noexcept;

    void diffuse_vorticity_PSE() noexcept;

    void advect_vortons(thread::Pool& pool) noexcept;

    void advect_tracers(thread::Pool& pool) noexcept;

    float3 compute_velocity(float3 const& position) const noexcept;

    AABB aabb_;

    float3 inv_extent_;

  public:
    static uint64_t constexpr Frame_length = scene::Units_per_second / 60;

  private:
    static float constexpr Time_step = static_cast<float>(
        static_cast<double>(Frame_length) / static_cast<double>(scene::Units_per_second));

    float viscosity_;

    float vorton_radius_;

    float tracer_radius_;

    Grid<float3> velocity_;

    Grid<float3x3> velocity_jacobian_;

    //	Grid<std::vector<uint32_t>>

    int3 visualization_dimensions_;

    Vorton*  vortons_;
    uint32_t num_vortons_;

    Particle* tracers_;
    uint32_t  num_tracers_;
};

}  // namespace procedural::fluid

#endif

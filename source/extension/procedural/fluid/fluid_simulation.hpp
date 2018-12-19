#ifndef SU_EXTENSION_PROCEDURAL_FLUID_SIMULATION_HPP
#define SU_EXTENSION_PROCEDURAL_FLUID_SIMULATION_HPP

#include "fluid_grid.hpp"

namespace procedural::fluid {

class Vorton;

class Simulation {
  public:
    Simulation(int3 const& dimensions) noexcept;

    ~Simulation() noexcept;

    float3 compute_velocity(float3 const& position) const noexcept;

  private:
    Grid<float3> velocity_;

    Vorton*  vortons_;
    uint32_t num_vortons_;
};

}  // namespace procedural::fluid

#endif

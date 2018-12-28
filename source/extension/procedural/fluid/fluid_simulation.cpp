#include "fluid_simulation.hpp"
#include "base/math/matrix3x3.inl"
#include "base/memory/align.hpp"
#include "fluid_particle.hpp"
#include "fluid_vorton.inl"
#include "base/math/aabb.inl"
#include "base/thread/thread_pool.hpp"
#include <vector>

namespace procedural::fluid {

static uint32_t constexpr Num_tracers = 51200000;
static uint32_t constexpr Num_vortons = 256;

void compute_jacobian(Grid<float3x3>& jacobian, Grid<float3> const& vec, float3 const& extent);

Simulation::Simulation(int3 const& dimensions) noexcept
    : aabb_(float3(-0.2f), float3(0.2f)),
      viscosity_(0.5f),
	  velocity_(dimensions),
      velocity_jacobian_(dimensions),
      vortons_(memory::allocate_aligned<Vorton>(Num_vortons)),
      num_vortons_(Num_vortons),
      tracers_(memory::allocate_aligned<Particle>(Num_tracers)),
      num_tracers_(Num_tracers)
{}

Simulation::~Simulation() noexcept {
    memory::free_aligned(vortons_);
}

void Simulation::simulate(thread::Pool& pool) noexcept {
    compute_velocity_grid(pool);

    stretch_and_tilt_vortons(pool);

	diffuse_vorticity_PSE();

	advect_vortons();

	advect_tracers(pool);
}

Vorton* Simulation::vortons() noexcept {
    return vortons_;
}

uint32_t Simulation::num_vortons() const noexcept {
    return num_vortons_;
}

Particle* Simulation::tracers() noexcept {
    return tracers_;
}

uint32_t Simulation::num_tracers() const noexcept {
    return num_tracers_;
}

float3 Simulation::world_to_texture_point(float3 const& v) const noexcept {
    return (v - aabb_.min()) / aabb_.extent();
}

void Simulation::compute_velocity_grid(thread::Pool& pool) noexcept {
    int3 const d = velocity_.dimensions();

    float3 const spacing = aabb_.extent() / float3(d);

	pool.run_range(
        [this, d, spacing](uint32_t /*id*/, int32_t begin, int32_t end) {
            for (int32_t z = begin; z < end; ++z) {
                for (int32_t y = 0; y < d[1]; ++y) {
                    for (int32_t x = 0; x < d[0]; ++x) {
                        int3 const xyz(x, y, z);

                        float3 const p(aabb_.min() + ((float3(xyz) + 0.5f) * spacing));

                        float3 const v = compute_velocity(p);

                        velocity_.set(xyz, v);
                    }
                }
            }
        },
        0, d[2]);
}

void Simulation::stretch_and_tilt_vortons(thread::Pool& pool) noexcept {
    float constexpr timeStep = 0.1f;

    compute_jacobian(velocity_jacobian_, velocity_, aabb_.extent());

	pool.run_range(
        [this, timeStep](uint32_t /*id*/, int32_t begin, int32_t end) {
            for (int32_t i = begin; i < end; ++i) {
                Vorton&  vorton = vortons_[i];

                float3x3 const vel_jac  = velocity_jacobian_.interpolate(
                    world_to_texture_point(vorton.position));

                float3 const stretch_tilt = transform_vector(vel_jac, vorton.vorticity);

                vorton.vorticity += /* fudge factor for stability */ 0.5f * timeStep * stretch_tilt;
            }
        },
        0, num_vortons_);
}

void Simulation::diffuse_vorticity_PSE() noexcept {
    float const timeStep = 0.1f;

    // Phase 1: Partition vortons

    // Create a spatial partition for the vortons.
    // Each cell contains a dynamic array of integers
    // whose values are offsets into mVortons.
    Grid<std::vector<uint32_t> > ugVortRef(velocity_.dimensions());
 //   ugVortRef.Init();

    for (uint32_t i = 0, len = num_vortons_; i < len; ++i) {  
        Vorton& rVorton = vortons_[i];
        // Insert the vorton's offset into the spatial partition.

		float3 const p = world_to_texture_point(rVorton.position);

			    if (p[0] < 0.f || p[0] >= 1.f || p[1] < 0.f || p[1] >= 1.f ||
                    p[2] < 0.f || p[2] >= 1.f) {
                    continue;
                }

        ugVortRef.at(p).push_back(i);
    }

    // Phase 2: Exchange vorticity with nearest neighbors

	int3 const d = ugVortRef.dimensions();

    const unsigned nx   = d[0];
    const unsigned  nxm1 = nx - 1;
    const unsigned ny   = d[1];
    const unsigned  nym1 = ny - 1;
    const unsigned  nxy  = nx * ny;
    const unsigned nz   = d[2];
    const unsigned  nzm1 = nz - 1;
    unsigned        idx[3];
    for (idx[2] = 0; idx[2] < nzm1; ++idx[2]) {  // For all points along z except the last...
        const unsigned offsetZ0 = idx[2] * nxy;
        const unsigned offsetZp = (idx[2] + 1) * nxy;
        for (idx[1] = 0; idx[1] < nym1; ++idx[1]) {  // For all points along y except the last...
            const unsigned offsetY0Z0 = idx[1] * nx + offsetZ0;
            const unsigned offsetYpZ0 = (idx[1] + 1) * nx + offsetZ0;
            const unsigned offsetY0Zp = idx[1] * nx + offsetZp;
            for (idx[0] = 0; idx[0] < nxm1;
                 ++idx[0]) {  // For all points along x except the last...
                const unsigned offsetX0Y0Z0 = idx[0] + offsetY0Z0;
                for (unsigned ivHere = 0; ivHere < ugVortRef.at(offsetX0Y0Z0).size();
                     ++ivHere) {  // For each vorton in this gridcell...
                    const unsigned& rVortIdxHere   = ugVortRef.at(offsetX0Y0Z0)[ivHere];
                    Vorton&         rVortonHere    = vortons_[rVortIdxHere];
                    float3&           rVorticityHere = rVortonHere.vorticity;

                    // Diffuse vorticity with other vortons in this same cell:
                    for (unsigned ivThere = ivHere + 1; ivThere < ugVortRef.at(offsetX0Y0Z0).size();
                         ++ivThere) {  // For each OTHER vorton within this same cell...
                        const unsigned& rVortIdxThere   = ugVortRef.at(offsetX0Y0Z0)[ivThere];
                        Vorton&           rVortonThere    = vortons_[rVortIdxThere];
                        float3&           rVorticityThere = rVortonThere.vorticity;
                        const float3    vortDiff        = rVorticityHere - rVorticityThere;
                        const float3      exchange =
                            2.0f * viscosity_ * timeStep *
                            vortDiff;  // Amount of vorticity to exchange between particles.
                        rVorticityHere -=
                            exchange;  // Make "here" vorticity a little closer to "there".
                        rVorticityThere +=
                            exchange;  // Make "there" vorticity a little closer to "here".
                    }

                    // Diffuse vorticity with vortons in adjacent cells:
                    {
                        const unsigned offsetXpY0Z0 =
                            idx[0] + 1 + offsetY0Z0;  // offset of adjacent cell in +X direction
                        for (unsigned ivThere = 0; ivThere < ugVortRef.at(offsetXpY0Z0).size();
                             ++ivThere) {  // For each vorton in the adjacent cell in +X
                                           // direction...
                            const unsigned& rVortIdxThere   = ugVortRef.at(offsetXpY0Z0)[ivThere];
                            Vorton&         rVortonThere    = vortons_[rVortIdxThere];
                            float3&         rVorticityThere = rVortonThere.vorticity;
                            const float3    vortDiff        = rVorticityHere - rVorticityThere;
                            const float3    exchange =
                                viscosity_ * timeStep *
                                vortDiff;  // Amount of vorticity to exchange between particles.
                            rVorticityHere -=
                                exchange;  // Make "here" vorticity a little closer to "there".
                            rVorticityThere +=
                                exchange;  // Make "there" vorticity a little closer to "here".
                        }
                    }

                    {
                        const unsigned offsetX0YpZ0 =
                            idx[0] + offsetYpZ0;  // offset of adjacent cell in +Y direction
                        for (unsigned ivThere = 0; ivThere < ugVortRef.at(offsetX0YpZ0).size();
                             ++ivThere) {  // For each vorton in the adjacent cell in +Y
                                           // direction...
                            const unsigned& rVortIdxThere   = ugVortRef.at(offsetX0YpZ0)[ivThere];
                            Vorton&         rVortonThere    = vortons_[rVortIdxThere];
                            float3&         rVorticityThere = rVortonThere.vorticity;
                            const float3    vortDiff        = rVorticityHere - rVorticityThere;
                            const float3    exchange =
                                viscosity_ * timeStep *
                                vortDiff;  // Amount of vorticity to exchange between particles.
                            rVorticityHere -=
                                exchange;  // Make "here" vorticity a little closer to "there".
                            rVorticityThere +=
                                exchange;  // Make "there" vorticity a little closer to "here".
                        }
                    }

                    {
                        const unsigned offsetX0Y0Zp =
                            idx[0] + offsetY0Zp;  // offset of adjacent cell in +Z direction
                        for (unsigned ivThere = 0; ivThere < ugVortRef.at(offsetX0Y0Zp).size();
                             ++ivThere) {  // For each vorton in the adjacent cell in +Z
                                           // direction...
                            const unsigned& rVortIdxThere   = ugVortRef.at(offsetX0Y0Zp)[ivThere];
                            Vorton&         rVortonThere    = vortons_[rVortIdxThere];
                            float3&         rVorticityThere = rVortonThere.vorticity;
                            const float3    vortDiff        = rVorticityHere - rVorticityThere;
                            const float3    exchange =
                                viscosity_ * timeStep *
                                vortDiff;  // Amount of vorticity to exchange between particles.
                            rVorticityHere -=
                                exchange;  // Make "here" vorticity a little closer to "there".
                            rVorticityThere +=
                                exchange;  // Make "there" vorticity a little closer to "here".
                        }
                    }

                    // Dissipate vorticity.  See notes in header comment.
                    rVorticityHere -= viscosity_ * timeStep *
                                      rVorticityHere;  // Reduce "here" vorticity.
                }
            }
        }
    }
}

void Simulation::advect_vortons() noexcept {
    float constexpr timeStep = 0.1f;

    for (uint32_t i = 0, len = num_vortons_; i < len; ++i) {  
        Vorton& rVorton = vortons_[i];
        float3 const velocity = velocity_.interpolate(world_to_texture_point(rVorton.position));
        rVorton.position += timeStep * velocity;
     //   rVorton.mVelocity = velocity;  // Cache this for use in collisions with rigid bodies.
    }
}

void Simulation::advect_tracers(thread::Pool& pool) noexcept {
    float constexpr timeStep = 0.1f;


	  pool.run_range(
        [this, timeStep](uint32_t /*id*/, int32_t begin, int32_t end) {
            for (int32_t i = begin; i < end; ++i) {
                Particle& tracer = tracers_[i];

                float3 const velocity = velocity_.interpolate(
                    world_to_texture_point(tracer.position));

                tracer.position += timeStep * velocity;

                //   rVorton.mVelocity = velocity;  // Cache this for use in collisions with rigid
                //   bodies.
            }
        },
        0, num_tracers_);
}

float3 Simulation::compute_velocity(float3 const& position) const noexcept {
    float3 velocity(0.f);

    for (uint32_t i = 0, len = num_vortons_; i < len; ++i) {
        velocity += vortons_[i].accumulate_velocity(position);
    }

    return velocity;
}

/*! \brief Compute Jacobian of a vector field

    \param jacobian - (output) UniformGrid of 3x3 matrix values.
                        The matrix is a vector of vectors.
                        Each component is a partial derivative with
                        respect to some direction:
                            j.a.b = d v.b / d a
                        where a and b are each one of {x,y,z}.
                        So j.x contains the partial derivatives with respect to x, etc.

    \param vec - UniformGrid of 3-vector values

*/
void compute_jacobian(Grid<float3x3>& jacobian, Grid<float3> const& vec, float3 const& extent) {
    const float3 spacing = extent / float3(vec.dimensions());

    const float3 reciprocalSpacing     = 1.f / spacing;
    const float3 halfReciprocalSpacing = 0.5f * reciprocalSpacing;

    const int3 dims       = vec.dimensions();
    const int3 dimsMinus1 = dims - 1;

    const int32_t numXY = dims[0] * dims[1];

    int32_t index[3];

#define ASSIGN_Z_OFFSETS                              \
    const unsigned offsetZM = numXY * (index[2] - 1); \
    const unsigned offsetZ0 = numXY * index[2];       \
    const unsigned offsetZP = numXY * (index[2] + 1);

#define ASSIGN_YZ_OFFSETS                                            \
    const unsigned offsetYMZ0 = dims[0] * (index[1] - 1) + offsetZ0; \
    const unsigned offsetY0Z0 = dims[0] * index[1] + offsetZ0;       \
    const unsigned offsetYPZ0 = dims[0] * (index[1] + 1) + offsetZ0; \
    const unsigned offsetY0ZM = dims[0] * index[1] + offsetZM;       \
    const unsigned offsetY0ZP = dims[0] * index[1] + offsetZP;

#define ASSIGN_XYZ_OFFSETS                                   \
    const unsigned offsetX0Y0Z0 = index[0] + offsetY0Z0;     \
    const unsigned offsetXMY0Z0 = index[0] - 1 + offsetY0Z0; \
    const unsigned offsetXPY0Z0 = index[0] + 1 + offsetY0Z0; \
    const unsigned offsetX0YMZ0 = index[0] + offsetYMZ0;     \
    const unsigned offsetX0YPZ0 = index[0] + offsetYPZ0;     \
    const unsigned offsetX0Y0ZM = index[0] + offsetY0ZM;     \
    const unsigned offsetX0Y0ZP = index[0] + offsetY0ZP;

    // Compute derivatives for interior (i.e. away from boundaries).
    for (index[2] = 1; index[2] < dimsMinus1[2]; ++index[2]) {
        ASSIGN_Z_OFFSETS;
        for (index[1] = 1; index[1] < dimsMinus1[1]; ++index[1]) {
            ASSIGN_YZ_OFFSETS;
            for (index[0] = 1; index[0] < dimsMinus1[0]; ++index[0]) {
                ASSIGN_XYZ_OFFSETS;

                float3x3& rMatrix = jacobian.data()[offsetX0Y0Z0];
                /* Compute d/dx */
                rMatrix.r[0] = halfReciprocalSpacing[0] *
                               (vec.at(offsetXPY0Z0) - vec.at(offsetXMY0Z0));
                /* Compute d/dy */
                rMatrix.r[1] = halfReciprocalSpacing[1] *
                               (vec.at(offsetX0YPZ0) - vec.at(offsetX0YMZ0));
                /* Compute d/dz */
                rMatrix.r[2] = halfReciprocalSpacing[2] *
                               (vec.at(offsetX0Y0ZP) - vec.at(offsetX0Y0ZM));
            }
        }
    }

    // Compute derivatives for boundaries: 6 faces of box.
    // In some situations, these macros compute extraneous data.
    // A tiny bit more efficiency could be squeezed from this routine,
    // but it turns out to be well under 1% of the total expense.

#define COMPUTE_FINITE_DIFF                                                                      \
    float3x3& rMatrix = jacobian.data()[offsetX0Y0Z0];                                           \
    if (index[0] == 0) {                                                                         \
        rMatrix.r[0] = reciprocalSpacing[0] * (vec.at(offsetXPY0Z0) - vec.at(offsetX0Y0Z0));     \
    } else if (index[0] == dimsMinus1[0]) {                                                      \
        rMatrix.r[0] = reciprocalSpacing[0] * (vec.at(offsetX0Y0Z0) - vec.at(offsetXMY0Z0));     \
    } else {                                                                                     \
        rMatrix.r[0] = halfReciprocalSpacing[0] * (vec.at(offsetXPY0Z0) - vec.at(offsetXMY0Z0)); \
    }                                                                                            \
    if (index[1] == 0) {                                                                         \
        rMatrix.r[1] = reciprocalSpacing[1] * (vec.at(offsetX0YPZ0) - vec.at(offsetX0Y0Z0));     \
    } else if (index[1] == dimsMinus1[1]) {                                                      \
        rMatrix.r[1] = reciprocalSpacing[1] * (vec.at(offsetX0Y0Z0) - vec.at(offsetX0YMZ0));     \
    } else {                                                                                     \
        rMatrix.r[1] = halfReciprocalSpacing[1] * (vec.at(offsetX0YPZ0) - vec.at(offsetX0YMZ0)); \
    }                                                                                            \
    if (index[2] == 0) {                                                                         \
        rMatrix.r[2] = reciprocalSpacing[2] * (vec.at(offsetX0Y0ZP) - vec.at(offsetX0Y0Z0));     \
    } else if (index[2] == dimsMinus1[2]) {                                                      \
        rMatrix.r[2] = reciprocalSpacing[2] * (vec.at(offsetX0Y0Z0) - vec.at(offsetX0Y0ZM));     \
    } else {                                                                                     \
        rMatrix.r[2] = halfReciprocalSpacing[2] * (vec.at(offsetX0Y0ZP) - vec.at(offsetX0Y0ZM)); \
    }

    // Compute derivatives for -X boundary.
    index[0] = 0;
    for (index[2] = 0; index[2] < dims[2]; ++index[2]) {
        ASSIGN_Z_OFFSETS;
        for (index[1] = 0; index[1] < dims[1]; ++index[1]) {
            ASSIGN_YZ_OFFSETS;
            {
                ASSIGN_XYZ_OFFSETS;
                COMPUTE_FINITE_DIFF;
            }
        }
    }

    // Compute derivatives for -Y boundary.
    index[1] = 0;
    for (index[2] = 0; index[2] < dims[2]; ++index[2]) {
        ASSIGN_Z_OFFSETS;
        {
            ASSIGN_YZ_OFFSETS;
            for (index[0] = 0; index[0] < dims[0]; ++index[0]) {
                ASSIGN_XYZ_OFFSETS;
                COMPUTE_FINITE_DIFF;
            }
        }
    }

    // Compute derivatives for -Z boundary.
    index[2] = 0;
    {
        ASSIGN_Z_OFFSETS;
        for (index[1] = 0; index[1] < dims[1]; ++index[1]) {
            ASSIGN_YZ_OFFSETS;
            for (index[0] = 0; index[0] < dims[0]; ++index[0]) {
                ASSIGN_XYZ_OFFSETS;
                COMPUTE_FINITE_DIFF;
            }
        }
    }

    // Compute derivatives for +X boundary.
    index[0] = dimsMinus1[0];
    for (index[2] = 0; index[2] < dims[2]; ++index[2]) {
        ASSIGN_Z_OFFSETS;
        for (index[1] = 0; index[1] < dims[1]; ++index[1]) {
            ASSIGN_YZ_OFFSETS;
            {
                ASSIGN_XYZ_OFFSETS;
                COMPUTE_FINITE_DIFF;
            }
        }
    }

    // Compute derivatives for +Y boundary.
    index[1] = dimsMinus1[1];
    for (index[2] = 0; index[2] < dims[2]; ++index[2]) {
        ASSIGN_Z_OFFSETS;
        {
            ASSIGN_YZ_OFFSETS;
            for (index[0] = 0; index[0] < dims[0]; ++index[0]) {
                ASSIGN_XYZ_OFFSETS;
                COMPUTE_FINITE_DIFF;
            }
        }
    }

    // Compute derivatives for +Z boundary.
    index[2] = dimsMinus1[2];
    {
        ASSIGN_Z_OFFSETS;
        for (index[1] = 0; index[1] < dims[1]; ++index[1]) {
            ASSIGN_YZ_OFFSETS;
            for (index[0] = 0; index[0] < dims[0]; ++index[0]) {
                ASSIGN_XYZ_OFFSETS;
                COMPUTE_FINITE_DIFF;
            }
        }
    }

#undef COMPUTE_FINITE_DIFF
#undef ASSIGN_XYZ_OFFSETS
#undef ASSIGN_YZ_OFFSETS
#undef ASSIGN_Z_OFFSETS
}

}  // namespace procedural::fluid

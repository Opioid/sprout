#include "fluid_simulation.hpp"
#include <vector>
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/ray.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "fluid_particle.hpp"
#include "fluid_vorton.inl"

#include <iostream>
#include "base/debug/assert.hpp"
#include "base/math/print.hpp"

namespace procedural::fluid {

static uint32_t constexpr Num_tracers = 51200000;
static uint32_t constexpr Num_vortons = 1024;

static float constexpr Vort_clamp = 1e4f;
static float constexpr Vel_clamp  = 1e3f;

void compute_jacobian(Grid<float3x3>& jacobian, Grid<float3> const& vec, float3 const& extent,
                      thread::Pool& pool);

Simulation::Simulation(int3 const& dimensions, int3 const& visualization_dimensions) noexcept
    : viscosity_(0.025f),
      vorton_radius_(0.005f),
      tracer_radius_(0.f),
      velocity_(dimensions),
      velocity_jacobian_(dimensions),
      visualization_dimensions_(visualization_dimensions),
      vortons_(memory::allocate_aligned<Vorton>(Num_vortons)),
      num_vortons_(Num_vortons),
      tracers_(memory::allocate_aligned<Particle>(Num_tracers)),
      num_tracers_(Num_tracers) {}

Simulation::~Simulation() noexcept {
    memory::free_aligned(tracers_);
    memory::free_aligned(vortons_);
}

void Simulation::set_aabb(AABB const& aabb) noexcept {
    aabb_ = aabb;

    float3 const padding = 0.5f * (aabb_.extent() / float3(velocity_.dimensions()));

    padded_aabb_ = AABB(aabb.min() + padding, aabb.max() - padding);

    inv_extent_ = 1.f / aabb_.extent();

    tracer_radius_ = 0.5f * (aabb_.extent()[0] / static_cast<float>(visualization_dimensions_[0]));
}

void Simulation::simulate(thread::Pool& pool) noexcept {
    compute_velocity_grid(pool);

    stretch_and_tilt_vortons(pool);

    diffuse_vorticity_PSE();

    advect_vortons(pool);

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

    //   float3 const a = (v - aabb_.min()) / aabb_.extent();
    //   float3 const b = (v - aabb_.min()) * inv_extent_;

    //  return a;
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
    compute_jacobian(velocity_jacobian_, velocity_, aabb_.extent(), pool);

    pool.run_range(
        [this](uint32_t /*id*/, int32_t begin, int32_t end) {
            for (int32_t i = begin; i < end; ++i) {
                Vorton& vorton = vortons_[i];

                //                if (!aabb_.intersect(vorton.position)) {
                //                    vorton.vorticity = float3(0.f);
                //                    continue;
                //                }

                float3x3 const vel_jac = velocity_jacobian_.interpolate(
                    world_to_texture_point(vorton.position));

                //                float3x3 const vel_jac = clamp(
                //                    velocity_jacobian_.interpolate(world_to_texture_point(vorton.position)),
                //                    -Vort_clamp, Vort_clamp);

                float3 const stretch_tilt = transform_vector(vel_jac, vorton.vorticity);

                //    vorton.vorticity += 0.5f * Time_step * stretch_tilt;

                vorton.vorticity = clamp(vorton.vorticity + 0.5f * Time_step * stretch_tilt,
                                         -Vort_clamp, Vort_clamp);

                SOFT_ASSERT(all_finite(vorton.vorticity));

                if (!all_finite(vorton.vorticity)) {
                    std::cout << "t(position): " << world_to_texture_point(vorton.position)
                              << std::endl;

                    std::cout << "stretch_tilt: " << stretch_tilt << std::endl;

                    std::cout << "vel_jac: " << vel_jac << std::endl;
                }
            }
        },
        0, num_vortons_);
}

void Simulation::diffuse_vorticity_PSE() noexcept {
    // Phase 1: Partition vortons

    // Create a spatial partition for the vortons.
    // Each cell contains a dynamic array of integers
    // whose values are offsets into mVortons.
    Grid<std::vector<uint32_t>> ugVortRef(velocity_.dimensions());
    //   ugVortRef.Init();

    for (uint32_t i = 0, len = num_vortons_; i < len; ++i) {
        Vorton const& vorton = vortons_[i];
        // Insert the vorton's offset into the spatial partition.

        float3 const p = world_to_texture_point(vorton.position);

        ugVortRef.at(p).push_back(i);
    }

    // Phase 2: Exchange vorticity with nearest neighbors

    int3 const d = ugVortRef.dimensions();

    uint32_t const nx   = d[0];
    uint32_t const nxm1 = nx - 1;
    uint32_t const ny   = d[1];
    uint32_t const nym1 = ny - 1;
    uint32_t const nxy  = nx * ny;
    uint32_t const nz   = d[2];
    uint32_t const nzm1 = nz - 1;
    uint32_t       idx[3];

    for (idx[2] = 0; idx[2] < nzm1; ++idx[2]) {
        uint32_t const offsetZ0 = idx[2] * nxy;
        uint32_t const offsetZp = (idx[2] + 1) * nxy;

        for (idx[1] = 0; idx[1] < nym1; ++idx[1]) {
            uint32_t const offsetY0Z0 = idx[1] * nx + offsetZ0;
            uint32_t const offsetYpZ0 = (idx[1] + 1) * nx + offsetZ0;
            uint32_t const offsetY0Zp = idx[1] * nx + offsetZp;

            for (idx[0] = 0; idx[0] < nxm1; ++idx[0]) {
                auto const& vortRefX0Y0Z0 = ugVortRef.at(idx[0] + offsetY0Z0);

                for (uint32_t ivHere = 0; ivHere < vortRefX0Y0Z0.size(); ++ivHere) {
                    uint32_t const vortIdxHere = vortRefX0Y0Z0[ivHere];

                    float3& vorticityHere = vortons_[vortIdxHere].vorticity;

                    // Diffuse vorticity with other vortons in this same cell:
                    for (uint32_t ivThere = ivHere + 1; ivThere < vortRefX0Y0Z0.size(); ++ivThere) {
                        uint32_t const vortIdxThere = vortRefX0Y0Z0[ivThere];

                        Vorton& vortonThere    = vortons_[vortIdxThere];
                        float3& vorticityThere = vortonThere.vorticity;

                        float3 const vortDiff = vorticityHere - vorticityThere;
                        float3 const exchange = 2.f * viscosity_ * Time_step * vortDiff;

                        vorticityHere -= exchange;
                        vorticityThere += exchange;
                    }

                    // Diffuse vorticity with vortons in adjacent cells:
                    {
                        auto const& vortRefXpY0Z0 = ugVortRef.at(idx[0] + 1 + offsetY0Z0);

                        for (uint32_t ivThere = 0; ivThere < vortRefXpY0Z0.size(); ++ivThere) {
                            uint32_t const vortIdxThere = vortRefXpY0Z0[ivThere];

                            float3& vorticityThere = vortons_[vortIdxThere].vorticity;

                            float3 const vortDiff = vorticityHere - vorticityThere;
                            float3 const exchange = viscosity_ * Time_step * vortDiff;

                            vorticityHere -= exchange;
                            vorticityThere += exchange;
                        }
                    }

                    {
                        auto const& vortRefX0YpZ0 = ugVortRef.at(idx[0] + offsetYpZ0);

                        for (uint32_t ivThere = 0; ivThere < vortRefX0YpZ0.size(); ++ivThere) {
                            uint32_t const vortIdxThere = vortRefX0YpZ0[ivThere];

                            float3& vorticityThere = vortons_[vortIdxThere].vorticity;

                            float3 const vortDiff = vorticityHere - vorticityThere;
                            float3 const exchange = viscosity_ * Time_step * vortDiff;

                            vorticityHere -= exchange;
                            vorticityThere += exchange;
                        }
                    }

                    {
                        auto const& vortRefX0Y0Zp = ugVortRef.at(idx[0] + offsetY0Zp);

                        for (uint32_t ivThere = 0; ivThere < vortRefX0Y0Zp.size(); ++ivThere) {
                            uint32_t const vortIdxThere = vortRefX0Y0Zp[ivThere];

                            float3& vorticityThere = vortons_[vortIdxThere].vorticity;

                            float3 const vortDiff = vorticityHere - vorticityThere;
                            float3 const exchange = viscosity_ * Time_step * vortDiff;

                            vorticityHere -= exchange;
                            vorticityThere += exchange;
                        }
                    }

                    // Dissipate vorticity.  See notes in header comment.
                    vorticityHere -= viscosity_ * Time_step * vorticityHere;
                }
            }
        }
    }
}

void Simulation::advect_vortons(thread::Pool& pool) noexcept {
    pool.run_range(
        [this](uint32_t /*id*/, int32_t begin, int32_t end) {
            float const radius = vorton_radius_;

            for (int32_t i = begin; i < end; ++i) {
                Vorton& vorton = vortons_[i];

                //    SOFT_ASSERT(all_finite(vorton.position));

                float3 const velocity = velocity_.interpolate(
                    world_to_texture_point(vorton.position));

                //      float3 const dir = Time_step * clamp(velocity, -Vel_clamp, Vel_clamp);
                float3 const dir = Time_step * velocity;

                //     SOFT_ASSERT(all_finite(dir));

                float const magnitude = length(dir);

                math::ray const ray(vorton.position, dir / magnitude, 0.f, magnitude + radius);

                if (float hit_t; padded_aabb_.intersect_p(ray, hit_t) && hit_t < ray.max_t) {
                    float3 const contact = ray.point(hit_t);

                    float3 const ambient = velocity_.interpolate(world_to_texture_point(contact));

                    float3 const vel_due_to_vort = vorton.accumulate_velocity(contact, radius);

                    float3 const vel_flow = ambient - vel_due_to_vort;
                    //      float3 const vel_flow = clamp(ambient - vel_due_to_vort, -Vel_clamp,
                    //      Vel_clamp);

                    float3 const normal = padded_aabb_.normal(contact);

                    float3 const vel_dir  = normalize(vel_flow);
                    float3 const vort_dir = cross(normal, vel_dir);
                    float3 const bend_dir = normalize(cross(vort_dir, vel_dir));

                    vorton.position = contact - radius * bend_dir;

                    if (!aabb_.intersect(vorton.position)) {
                        std::cout << "Alarm 0" << std::endl;
                    }

                    //     std::cout << max_component(abs(vorton.position)) << std::endl;

                    float3 const old_vorticity = vorton.vorticity;

                    vorton.assign_by_velocity(contact, -vel_flow, radius);

                    float constexpr Gain           = 0.1f;
                    float constexpr One_minus_gain = 1.f - Gain;

                    //                    vorton.vorticity = clamp(
                    //                        Gain * vorton.vorticity + One_minus_gain *
                    //                        old_vorticity, -Vel_clamp, Vel_clamp);

                    vorton.vorticity = Gain * vorton.vorticity + One_minus_gain * old_vorticity;

                    if (!all_finite(vorton.vorticity)) {
                        std::cout << "ambient " << ambient << std::endl;
                        std::cout << "vel_due_to_vort " << vel_due_to_vort << std::endl;
                        std::cout << "vel_flow " << vel_flow << std::endl;
                        std::cout << "vorticity " << vorton.vorticity << std::endl;
                        std::cout << "old vorticity " << old_vorticity << std::endl;
                    }

                    SOFT_ASSERT(all_finite(vorton.position));
                } else {
                    vorton.position += dir;

                    if (!aabb_.intersect(vorton.position)) {
                        std::cout << "Alarm 1" << std::endl;
                    }

                    SOFT_ASSERT(all_finite(vorton.position));
                }
            }
        },
        0, num_vortons_);
}

void Simulation::advect_tracers(thread::Pool& pool) noexcept {
    pool.run_range(
        [this](uint32_t /*id*/, int32_t begin, int32_t end) {
            float const radius = tracer_radius_;

            for (int32_t i = begin; i < end; ++i) {
                Particle& tracer = tracers_[i];

                float3 const velocity = velocity_.interpolate(
                    world_to_texture_point(tracer.position));

                float3 const dir = Time_step * velocity;

                float const magnitude = length(dir);

                math::ray const ray(tracer.position, dir / magnitude, 0.f, magnitude);

                if (float hit_t; aabb_.intersect_p(ray, hit_t) && hit_t < ray.max_t) {
                    tracer.position = ray.point(hit_t - radius);
                } else {
                    tracer.position += dir;
                }
            }
        },
        0, num_tracers_);
}

float3 Simulation::compute_velocity(float3 const& position) const noexcept {
    float3 velocity(0.f);

    float const radius = vorton_radius_;

    for (uint32_t i = 0, len = num_vortons_; i < len; ++i) {
        //   if (aabb_.intersect(vortons_[i].position)) {
        velocity += vortons_[i].accumulate_velocity(position, radius);
        //   }
    }

    //  return clamp(velocity, -Vel_clamp, Vel_clamp);

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
void compute_jacobian(Grid<float3x3>& jacobian, Grid<float3> const& vec, float3 const& extent,
                      thread::Pool& pool) {
    float3 const spacing = extent / float3(vec.dimensions());

    float3 const reciprocalSpacing     = 1.f / spacing;
    float3 const halfReciprocalSpacing = 0.5f * reciprocalSpacing;

    int3 const dims       = vec.dimensions();
    int3 const dimsMinus1 = dims - 1;

    int32_t const numXY = dims[0] * dims[1];

    int32_t index[3];

#define ASSIGN_Z_OFFSETS                              \
    uint32_t const offsetZM = numXY * (index[2] - 1); \
    uint32_t const offsetZ0 = numXY * index[2];       \
    uint32_t const offsetZP = numXY * (index[2] + 1);

#define ASSIGN_YZ_OFFSETS                                            \
    uint32_t const offsetYMZ0 = dims[0] * (index[1] - 1) + offsetZ0; \
    uint32_t const offsetY0Z0 = dims[0] * index[1] + offsetZ0;       \
    uint32_t const offsetYPZ0 = dims[0] * (index[1] + 1) + offsetZ0; \
    uint32_t const offsetY0ZM = dims[0] * index[1] + offsetZM;       \
    uint32_t const offsetY0ZP = dims[0] * index[1] + offsetZP;

#define ASSIGN_XYZ_OFFSETS                                   \
    uint32_t const offsetX0Y0Z0 = index[0] + offsetY0Z0;     \
    uint32_t const offsetXMY0Z0 = index[0] - 1 + offsetY0Z0; \
    uint32_t const offsetXPY0Z0 = index[0] + 1 + offsetY0Z0; \
    uint32_t const offsetX0YMZ0 = index[0] + offsetYMZ0;     \
    uint32_t const offsetX0YPZ0 = index[0] + offsetYPZ0;     \
    uint32_t const offsetX0Y0ZM = index[0] + offsetY0ZM;     \
    uint32_t const offsetX0Y0ZP = index[0] + offsetY0ZP;

    // Compute derivatives for interior (i.e. away from boundaries).

    pool.run_range(
        [&jacobian, &vec, halfReciprocalSpacing, dims, dimsMinus1, numXY](
            uint32_t /*id*/, int32_t begin, int32_t end) {
            int32_t index[3];
            for (index[2] = begin; index[2] < end; ++index[2]) {
                ASSIGN_Z_OFFSETS;
                for (index[1] = 1; index[1] < dimsMinus1[1]; ++index[1]) {
                    ASSIGN_YZ_OFFSETS;
                    for (index[0] = 1; index[0] < dimsMinus1[0]; ++index[0]) {
                        ASSIGN_XYZ_OFFSETS;

                        float3x3& rMatrix = jacobian.data()[offsetX0Y0Z0];

                        // Compute d/dx
                        rMatrix.r[0] = halfReciprocalSpacing[0] *
                                       (vec.at(offsetXPY0Z0) - vec.at(offsetXMY0Z0));

                        // Compute d/dy
                        rMatrix.r[1] = halfReciprocalSpacing[1] *
                                       (vec.at(offsetX0YPZ0) - vec.at(offsetX0YMZ0));

                        // Compute d/dz
                        rMatrix.r[2] = halfReciprocalSpacing[2] *
                                       (vec.at(offsetX0Y0ZP) - vec.at(offsetX0Y0ZM));
                    }
                }
            }
        },
        1, dimsMinus1[2]);

    /*
for (index[2] = 1; index[2] < dimsMinus1[2]; ++index[2]) {
    ASSIGN_Z_OFFSETS;
    for (index[1] = 1; index[1] < dimsMinus1[1]; ++index[1]) {
        ASSIGN_YZ_OFFSETS;
        for (index[0] = 1; index[0] < dimsMinus1[0]; ++index[0]) {
            ASSIGN_XYZ_OFFSETS;

            float3x3& rMatrix = jacobian.data()[offsetX0Y0Z0];

                            // Compute d/dx
            rMatrix.r[0] = halfReciprocalSpacing[0] *
                           (vec.at(offsetXPY0Z0) - vec.at(offsetXMY0Z0));

                            // Compute d/dy
            rMatrix.r[1] = halfReciprocalSpacing[1] *
                           (vec.at(offsetX0YPZ0) - vec.at(offsetX0YMZ0));

                            // Compute d/dz.
            rMatrix.r[2] = halfReciprocalSpacing[2] *
                           (vec.at(offsetX0Y0ZP) - vec.at(offsetX0Y0ZM));
        }
    }
}
*/
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

    // Mega kernel

    pool.run_range(
        [&jacobian, &vec, reciprocalSpacing, halfReciprocalSpacing, dims, dimsMinus1, numXY](
            uint32_t /*id*/, int32_t begin, int32_t end) {
            int32_t index[3];
            for (index[2] = begin; index[2] < end; ++index[2]) {
                ASSIGN_Z_OFFSETS;

                // Compute derivatives for -X boundary.
                index[0] = 0;
                for (index[1] = 0; index[1] < dims[1]; ++index[1]) {
                    ASSIGN_YZ_OFFSETS;
                    {
                        ASSIGN_XYZ_OFFSETS;
                        COMPUTE_FINITE_DIFF;
                    }
                }

                // Compute derivatives for -Y boundary.
                index[1] = 0;
                {
                    ASSIGN_YZ_OFFSETS;
                    for (index[0] = 0; index[0] < dims[0]; ++index[0]) {
                        ASSIGN_XYZ_OFFSETS;
                        COMPUTE_FINITE_DIFF;
                    }
                }

                // Compute derivatives for +X boundary.
                index[0] = dimsMinus1[0];

                for (index[1] = 0; index[1] < dims[1]; ++index[1]) {
                    ASSIGN_YZ_OFFSETS;
                    {
                        ASSIGN_XYZ_OFFSETS;
                        COMPUTE_FINITE_DIFF;
                    }
                }

                // Compute derivatives for +Y boundary.
                index[1] = dimsMinus1[1];

                {
                    ASSIGN_YZ_OFFSETS;
                    for (index[0] = 0; index[0] < dims[0]; ++index[0]) {
                        ASSIGN_XYZ_OFFSETS;
                        COMPUTE_FINITE_DIFF;
                    }
                }
            }
        },
        0, dims[2]);

    /*
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
    */

    /*
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
    */

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

    /*
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
    */

    /*
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
    */

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

#ifndef SU_CORE_SCENE_PROP_PROP_HPP
#define SU_CORE_SCENE_PROP_PROP_HPP

#include <vector>
#include "base/math/aabb.hpp"
#include "scene/entity/entity.hpp"
#include "scene/material/material.hpp"

namespace thread {
class Pool;
}

namespace scene {

class Worker;
struct Ray;

namespace shape {

struct Intersection;
class Shape;
class Node_stack;

}  // namespace shape

namespace prop {

class Prop : public entity::Entity {
  public:
    using Node_stack = shape::Node_stack;
    using Filter     = material::Sampler_settings::Filter;
    using Shape      = shape::Shape;

    ~Prop() noexcept override;

    void morph(thread::Pool& pool) noexcept;

    bool intersect(Ray& ray, Node_stack& node_stack, shape::Intersection& intersection) const
        noexcept;

    bool intersect_fast(Ray& ray, Node_stack& node_stack, shape::Intersection& intersection) const
        noexcept;

    bool intersect(Ray& ray, Node_stack& node_stack, float& epsilon) const noexcept;

    bool intersect_p(Ray const& ray, shape::Node_stack& node_stack) const noexcept;

    //	bool intersect_p(FVector ray_origin, FVector ray_direction,
    //					 FVector ray_inv_direction, FVector ray_mint_, FVector
    // ray_max_t, 					 float ray_time, shape::Node_stack&
    // node_stack) const;

    shape::Shape const* shape() const noexcept;
    shape::Shape*       shape() noexcept;

    AABB const& aabb() const noexcept;

    void set_shape_and_materials(Shape* shape, Materials const& materials) noexcept;

    void set_parameters(json::Value const& parameters) noexcept override;

    void prepare_sampling(uint32_t part, uint32_t light_id, uint64_t time,
                          bool material_importance_sampling, thread::Pool& pool) noexcept;

    void prepare_sampling_volume(uint32_t part, uint32_t light_id, uint64_t time,
                                 bool material_importance_sampling, thread::Pool& pool) noexcept;

    float opacity(Ray const& ray, Filter filter, Worker const& worker) const noexcept;

    float3 thin_absorption(Ray const& ray, Filter filter, Worker const& worker) const noexcept;

    float area(uint32_t part) const noexcept;

    float volume(uint32_t part) const noexcept;

    uint32_t light_id(uint32_t part) const noexcept;

    material::Material const* material(uint32_t part) const noexcept;

    bool has_masked_material() const noexcept;
    bool has_caustic_material() const noexcept;
    bool has_tinted_shadow() const noexcept;
    bool has_no_surface() const noexcept;

    size_t num_bytes() const noexcept;

  protected:
    void set_shape(Shape* shape) noexcept;

    bool visible(uint32_t ray_depth) const noexcept;

    void on_set_transformation() noexcept override final;

    // Pre-transformed AABB in world space.
    // For moving objects it must cover the entire area occupied by the object during the tick.
    AABB aabb_;

    Shape* shape_;

    struct Part {
        union {
            float area;
            float volume;
        };

        uint32_t light_id;
    };

    std::vector<Part> parts_;

    Materials materials_;
};

}  // namespace prop

using Prop = prop::Prop;

}  // namespace scene

#endif

#ifndef SU_CORE_SCENE_PROP_PROP_HPP
#define SU_CORE_SCENE_PROP_PROP_HPP

#include "scene/body/body.hpp"
#include "scene/material/material.hpp"
#include "base/math/aabb.hpp"
#include <memory>
#include <vector>

namespace thread { class Pool; }

namespace scene {

class Worker;
struct Ray;

namespace shape {

struct Intersection;
class Shape;
class Node_stack;

}

namespace prop {

class Prop final : public body::Body {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	virtual ~Prop();

	void set_shape_and_materials(const std::shared_ptr<shape::Shape>& shape,
								 const material::Materials& materials);

	virtual void set_parameters(const json::Value& parameters) override final;

	void prepare_sampling(uint32_t part, uint32_t light_id,
						  bool material_importance_sampling, thread::Pool& pool);

	float opacity(const Ray& ray, Sampler_filter filter, const Worker& worker) const;

	float3 thin_absorption(const Ray& ray, Sampler_filter filter, const Worker& worker) const;

	float area(uint32_t part) const;
	uint32_t light_id(uint32_t part) const;
	material::Material* material(uint32_t part) const;

	bool has_masked_material() const;
	bool has_tinted_shadow() const;

	bool is_open() const;
	void set_open(bool open);

	size_t num_bytes() const;

private:

	struct Part {
		float    area;
		uint32_t light_id;
	};

	std::vector<Part> parts_;

	material::Materials materials_;
};

}}

#endif

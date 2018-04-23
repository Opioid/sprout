#ifndef SU_CORE_SCENE_CAMERA_CAMERA_HPP
#define SU_CORE_SCENE_CAMERA_CAMERA_HPP

#include "scene/entity/entity.hpp"
#include "scene/prop/interface_stack.hpp"
#include "base/math/vector2.hpp"
#include <memory>
#include <string>
#include <vector>

namespace sampler { struct Camera_sample; }

namespace rendering::sensor { class Sensor; }

namespace scene {

struct Ray;
class Scene;
class Worker;

namespace camera {

class Camera : public entity::Entity {

public:

	Camera(int2 resolution);
	virtual ~Camera() override;

	virtual uint32_t num_views() const = 0;

	virtual int2 sensor_dimensions() const = 0;

	virtual int4 view_bounds(uint32_t view) const = 0;

	virtual float pixel_solid_angle() const = 0;

	void update(const Scene& scene, Worker& worker);

	virtual bool generate_ray(const sampler::Camera_sample& sample,
							  uint32_t view, Ray& ray) const = 0;

	virtual void set_parameters(const json::Value& parameters) override final;

	int2 resolution() const;

	rendering::sensor::Sensor& sensor() const;
	void set_sensor(std::unique_ptr<rendering::sensor::Sensor> sensor);

	const prop::Interface_stack& interface_stack() const;

	float frame_duration() const;
	void set_frame_duration(float frame_duration);

	bool motion_blur() const;
	void set_motion_blur(bool motion_blur);

protected:

	virtual void on_update(Worker& worker) = 0;

	virtual void set_parameter(const std::string& name, const json::Value& value) = 0;

	virtual void on_set_transformation() override final;

	static Ray create_ray(const float3& origin, const float3& direction, float time);

	int2 resolution_;
	std::unique_ptr<rendering::sensor::Sensor> sensor_;

	prop::Interface_stack interface_stack_;

	int32_t filter_radius_ = 0;

	float frame_duration_ = 0.f;
	bool motion_blur_ = true;

	prop::Interface_stack interfaces_;
};

}}

#endif

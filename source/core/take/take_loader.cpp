#include "take_loader.hpp"
#include "take.hpp"
#include "rendering/film/unfiltered.hpp"
#include "rendering/integrator/surface/ao.hpp"
#include "scene/camera/perspective_camera.hpp"
#include "base/math/vector.inl"
#include "base/json/json.hpp"
#include <fstream>

namespace take {

std::shared_ptr<Take> Loader::load(const std::string& filename) {
	std::ifstream stream(filename, std::ios::binary);

	if (!stream) {
		return nullptr;
	}

	rapidjson::Document root;
	if (!json::parse(stream, root)) {
		return nullptr;
	}

	auto take = std::make_shared<Take>();

	for (auto n = root.MemberBegin(); n != root.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("scene" == node_name) {
			take->scene = node_value.GetString();
		} else if ("integrator" == node_name) {
			take->surface_integrator_factory = load_surface_integrator_factory();
		}
	}

	math::uint2 dimensions(512, 512);
	film::Film* film = new film::Unfiltered(dimensions);

	take->context.camera = std::make_shared<camera::Perspective>(film);

	return take;
}

std::shared_ptr<rendering::Surface_integrator_factory> Loader::load_surface_integrator_factory() {
	return std::make_shared<rendering::Ao_factory>();
}

}

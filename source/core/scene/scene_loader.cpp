#include "scene_loader.hpp"
#include "scene.hpp"
#include "scene/surrounding/uniform_surrounding.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include <fstream>

namespace scene {

bool Loader::load(const std::string& filename, Scene& scene) {
	std::ifstream stream(filename, std::ios::binary);
	if (!stream) {
		return false;
	}

	auto root = json::parse(stream);
	if (!root) {
		return false;
	}

	for (auto n = root->MemberBegin(); n != root->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("surrounding" == node_name) {
			surrounding::Surrounding* surrounding = load_surrounding(node_value);
			scene.set_surrounding(surrounding);
		} else if ("entities" == node_name) {

		}
	}

	return true;
}

surrounding::Surrounding* Loader::load_surrounding(const rapidjson::Value& surrounding_value) const {
	std::string type_name = json::read_string(surrounding_value, "type");

	if ("Uniform" == type_name) {
		math::float3 energy = json::read_float3(surrounding_value, "color");
		surrounding::Uniform* uniform = new surrounding::Uniform(energy);
		return uniform;
	} else if ("Textured" == type_name) {

	}

	return nullptr;
}

}

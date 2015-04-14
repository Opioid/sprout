#include "material_provider.hpp"
#include "material_sample_cache.inl"
#include "substitute/substitute_constant.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include <fstream>
#include <iostream>

namespace scene { namespace material {

Provider::Provider(uint32_t num_workers) :
	substitute_cache_(num_workers),
	fallback_material_(std::make_shared<substitute::Constant>(substitute_cache_, math::float3(1.f, 0.f, 0.f), 1.f, 0.f)) {}

std::shared_ptr<IMaterial> Provider::load(const std::string& filename, uint32_t flags) {
	std::ifstream stream(filename, std::ios::binary);
	if (!stream) {
		return nullptr;
	}

	auto root = json::parse(stream);

	// checking for positions now, but handling them later
	const rapidjson::Value::ConstMemberIterator rendering_node = root->FindMember("rendering");
	if (root->MemberEnd() == rendering_node) {
		return nullptr;
	}

	const rapidjson::Value& rendering_value = rendering_node->value;

	for (auto n = rendering_value.MemberBegin(); n != rendering_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("Substitute" == node_name) {
			return load_substitute(node_value);
		}
	}

	return nullptr;
}

std::shared_ptr<IMaterial> Provider::fallback_material() const {
	return fallback_material_;
}

std::shared_ptr<IMaterial> Provider::load_substitute(const rapidjson::Value& substitute_value) {
	math::float3 color(0.75f, 0.75f, 0.75f);
	float roughness = 0.9f;
	float metallic = 0.f;

	for (auto n = substitute_value.MemberBegin(); n != substitute_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("color" == node_name) {
			color = json::read_float3(node_value);
		} else if ("roughness" == node_name) {
			roughness = json::read_float(node_value);
		} else if ("metallic" == node_name) {
			metallic = json::read_float(node_value);
		}
	}

	return std::make_shared<substitute::Constant>(substitute_cache_, color, roughness, metallic);
}

}}

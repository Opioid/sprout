#include "material_provider.hpp"
#include "material_sample_cache.inl"
#include "image/image_provider.hpp"
#include "substitute/substitute_colormap.hpp"
#include "substitute/substitute_colormap_normalmap.hpp"
#include "substitute/substitute_constant.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include <fstream>
#include <iostream>

namespace scene { namespace material {

Provider::Provider(resource::Cache<image::Image>& image_cache, uint32_t num_workers) :
	image_cache_(image_cache),
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
	std::shared_ptr<image::Image> colormap;
	std::shared_ptr<image::Image> normalmap;

	for (auto n = substitute_value.MemberBegin(); n != substitute_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("color" == node_name) {
			color = json::read_float3(node_value);
		} else if ("roughness" == node_name) {
			roughness = json::read_float(node_value);
		} else if ("metallic" == node_name) {
			metallic = json::read_float(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				std::string filename = json::read_string(*tn, "file", "");
				std::string usage    = json::read_string(*tn, "usage", "Color");

				if (filename.empty()) {
					continue;
				}

				if ("Color" == usage) {
					colormap = image_cache_.load(filename);
				} else if ("Normal" == usage) {
					normalmap = image_cache_.load(filename, static_cast<uint32_t>(image::Provider::Flags::Use_as_normal));
				}
			}
		}
	}

	if (colormap) {
		if (normalmap) {
			return std::make_shared<substitute::Colormap_normalmap>(substitute_cache_, colormap, normalmap, roughness, metallic);
		} else {
			return std::make_shared<substitute::Colormap>(substitute_cache_, colormap, roughness, metallic);
		}
	}

	return std::make_shared<substitute::Constant>(substitute_cache_, color, roughness, metallic);
}

}}

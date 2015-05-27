#include "material_provider.hpp"
#include "material_sample_cache.inl"
#include "image/image_provider.hpp"
#include "glass/glass_constant.hpp"
#include "glass/glass_normalmap.hpp"
#include "light/light_constant.hpp"
#include "matte/matte_colormap.hpp"
#include "matte/matte_colormap_normalmap.hpp"
#include "matte/matte_constant.hpp"
#include "substitute/substitute_colormap.hpp"
#include "substitute/substitute_colormap_normalmap.hpp"
#include "substitute/substitute_colormap_normalmap_surfacemap.hpp"
#include "substitute/substitute_colormap_normalmap_surfacemap_emissionmap.hpp"
#include "substitute/substitute_colormap_surfacemap.hpp"
#include "substitute/substitute_constant.hpp"
#include "substitute/substitute_normalmap.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include <fstream>
#include <iostream>

namespace scene { namespace material {

Provider::Provider(resource::Cache<image::Image>& image_cache, uint32_t num_workers) :
	image_cache_(image_cache),
	glass_cache_(num_workers),
	light_cache_(num_workers),
	matte_cache_(num_workers),
	substitute_cache_(num_workers),
	fallback_material_(std::make_shared<substitute::Constant>(substitute_cache_, nullptr, math::float3(1.f, 0.f, 0.f), 1.f, 0.f)) {}

std::shared_ptr<IMaterial> Provider::load(const std::string& filename, uint32_t /*flags*/) {
	std::ifstream stream(filename, std::ios::binary);
	if (!stream) {
		throw std::runtime_error("File \"" + filename + "\" could not be opened");
	}

	auto root = json::parse(stream);

	// checking for positions now, but handling them later
	const rapidjson::Value::ConstMemberIterator rendering_node = root->FindMember("rendering");
	if (root->MemberEnd() == rendering_node) {
		throw std::runtime_error("Material \"" + filename + "\" has no render node");
	}

	const rapidjson::Value& rendering_value = rendering_node->value;

	for (auto n = rendering_value.MemberBegin(); n != rendering_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("Glass" == node_name) {
			return load_glass(node_value);
		} else if ("Light" == node_name) {
			return load_light(node_value);
		} else if ("Matte" == node_name) {
			return load_matte(node_value);
		} else if ("Substitute" == node_name) {
			return load_substitute(node_value);
		}
	}

	throw std::runtime_error("Material \"" + filename + "\" is of no known type");
}

std::shared_ptr<IMaterial> Provider::fallback_material() const {
	return fallback_material_;
}

std::shared_ptr<IMaterial> Provider::load_glass(const rapidjson::Value& glass_value) {
	math::float3 color(1.f, 1.f, 1.f);
	float attenuation_distance = 1.f;
	float ior = 1.5f;
	std::shared_ptr<image::Image> normalmap;

	for (auto n = glass_value.MemberBegin(); n != glass_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("color" == node_name) {
			color = json::read_float3(node_value);
		} else if ("attenuation_distance" == node_name) {
			attenuation_distance = json::read_float(node_value);
		} else if ("ior" == node_name) {
			ior = json::read_float(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				std::string filename = json::read_string(*tn, "file", "");
				std::string usage    = json::read_string(*tn, "usage", "Color");

				if (filename.empty()) {
					continue;
				}

				if ("Normal" == usage) {
					normalmap = image_cache_.load(filename, static_cast<uint32_t>(image::Provider::Flags::Use_as_normal));
				}
			}
		}
	}

	if (normalmap) {
		return std::make_shared<glass::Normalmap>(glass_cache_, nullptr, color, attenuation_distance, ior, normalmap);
	}

	return std::make_shared<glass::Constant>(glass_cache_, nullptr, color, attenuation_distance, ior);
}

std::shared_ptr<IMaterial> Provider::load_light(const rapidjson::Value& light_value) {
	math::float3 emission(10.f, 10.f, 10.f);

	for (auto n = light_value.MemberBegin(); n != light_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("emission" == node_name) {
			emission = json::read_float3(node_value);
		}
	}

	return std::make_shared<light::Constant>(light_cache_, nullptr, emission);
}

std::shared_ptr<IMaterial> Provider::load_matte(const rapidjson::Value& matte_value) {
	math::float3 color(0.75f, 0.75f, 0.75f);

	std::shared_ptr<image::Image> colormap;
	std::shared_ptr<image::Image> mask;

	for (auto n = matte_value.MemberBegin(); n != matte_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("color" == node_name) {
			color = json::read_float3(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				std::string filename = json::read_string(*tn, "file", "");
				std::string usage    = json::read_string(*tn, "usage", "Color");

				if (filename.empty()) {
					continue;
				}

				if ("Color" == usage) {
					colormap = image_cache_.load(filename);
				} else if ("Mask" == usage) {
					mask = image_cache_.load(filename, static_cast<uint32_t>(image::Provider::Flags::Use_as_mask));
				}
			}
		}
	}

	return std::make_shared<matte::Constant>(matte_cache_, mask, color);
}

std::shared_ptr<IMaterial> Provider::load_substitute(const rapidjson::Value& substitute_value) {
	math::float3 color(0.75f, 0.75f, 0.75f);
	float roughness = 0.9f;
	float metallic = 0.f;
	float emission_factor = 1.f;
	std::shared_ptr<image::Image> colormap;
	std::shared_ptr<image::Image> normalmap;
	std::shared_ptr<image::Image> surfacemap;
	std::shared_ptr<image::Image> emissionmap;
	std::shared_ptr<image::Image> mask;

	for (auto n = substitute_value.MemberBegin(); n != substitute_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("color" == node_name) {
			color = json::read_float3(node_value);
		} else if ("roughness" == node_name) {
			roughness = json::read_float(node_value);
		} else if ("metallic" == node_name) {
			metallic = json::read_float(node_value);
		} else if ("emission_factor" == node_name) {
			emission_factor = json::read_float(node_value);
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
				} else if ("Surface" == usage) {
					surfacemap = image_cache_.load(filename);
				} else if ("Emission" == usage) {
					emissionmap = image_cache_.load(filename);
				} else if ("Mask" == usage) {
					mask = image_cache_.load(filename, static_cast<uint32_t>(image::Provider::Flags::Use_as_mask));
				}
			}
		}
	}

	if (colormap) {
		if (normalmap) {
			if (surfacemap) {
				if (emissionmap) {
					return std::make_shared<substitute::Colormap_normalmap_surfacemap_emissionmap>(
								substitute_cache_, mask, colormap, normalmap, surfacemap, emissionmap, emission_factor, metallic);
				} else {
					return std::make_shared<substitute::Colormap_normalmap_surfacemap>(
								substitute_cache_, mask, colormap, normalmap, surfacemap, metallic);
				}
			} else {
				return std::make_shared<substitute::Colormap_normalmap>(substitute_cache_, mask, colormap, normalmap, roughness, metallic);
			}
		} else {
			if (surfacemap) {
				return std::make_shared<substitute::Colormap_surfacemap>(substitute_cache_, mask, colormap, surfacemap, metallic);
			} else {
				return std::make_shared<substitute::Colormap>(substitute_cache_, mask, colormap, roughness, metallic);
			}
		}
	} else {
		if (normalmap) {
			return std::make_shared<substitute::Normalmap>(substitute_cache_, mask, color, normalmap, roughness, metallic);
		}
	}

	return std::make_shared<substitute::Constant>(substitute_cache_, mask, color, roughness, metallic);

/*
	if (colormap) {
		if (normalmap) {
			return std::make_shared<matte::Colormap_normalmap>(matte_cache_, mask, colormap, normalmap);
		} else {
			return std::make_shared<matte::Colormap>(matte_cache_, mask, colormap);
		}
	}

	return std::make_shared<matte::Constant>(matte_cache_, mask, color);
*/
}

}}

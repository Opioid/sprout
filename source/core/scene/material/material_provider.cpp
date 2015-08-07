#include "material_provider.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_cache.inl"
#include "material_sample_cache.inl"
#include "image/texture/texture_2d_provider.hpp"
#include "glass/glass_constant.hpp"
#include "glass/glass_normalmap.hpp"
#include "light/light_constant.hpp"
#include "light/light_emissionmap.hpp"
#include "matte/matte_colormap.hpp"
#include "matte/matte_colormap_normalmap.hpp"
#include "matte/matte_constant.hpp"
#include "matte/matte_normalmap.hpp"
#include "substitute/substitute_colormap.hpp"
#include "substitute/substitute_colormap_normalmap.hpp"
#include "substitute/substitute_colormap_normalmap_surfacemap.hpp"
#include "substitute/substitute_colormap_normalmap_surfacemap_emissionmap.hpp"
#include "substitute/substitute_colormap_surfacemap.hpp"
#include "substitute/substitute_constant.hpp"
#include "substitute/substitute_normalmap.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material {

Provider::Provider(file::System& file_system, resource::Cache<image::texture::Texture_2D>& texture_cache, uint32_t num_workers) :
	resource::Provider<IMaterial>(file_system),
	texture_cache_(texture_cache),
	glass_cache_(num_workers),
	light_cache_(num_workers),
	matte_cache_(num_workers),
	substitute_cache_(num_workers),
	fallback_material_(std::make_shared<substitute::Constant>(substitute_cache_,
															  nullptr,
															  false,
															  math::float3(1.f, 0.f, 0.f), 1.f, 0.f)) {}

std::shared_ptr<IMaterial> Provider::load(const std::string& filename, uint32_t /*flags*/) {
	auto stream_pointer = file_system_.read_stream(filename);
	if (!*stream_pointer) {
		throw std::runtime_error("File \"" + filename + "\" could not be opened");
	}

	auto& stream = *stream_pointer;

	auto root = json::parse(stream);

	// checking for positions now, but handling them later
	const rapidjson::Value::ConstMemberIterator rendering_node = root->FindMember("rendering");
	if (root->MemberEnd() == rendering_node) {
		throw std::runtime_error("Material has no render node");
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

	throw std::runtime_error("Material is of unknown type");
}

std::shared_ptr<IMaterial> Provider::fallback_material() const {
	return fallback_material_;
}

std::shared_ptr<IMaterial> Provider::load_glass(const rapidjson::Value& glass_value) {
	math::float3 color(1.f, 1.f, 1.f);
	float attenuation_distance = 1.f;
	float ior = 1.5f;
	std::shared_ptr<image::texture::Texture_2D> normalmap;

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
					normalmap = texture_cache_.load(filename, static_cast<uint32_t>(image::texture::Provider::Flags::Use_as_normal));
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
	float emission_factor = 1.f;

	std::shared_ptr<image::texture::Texture_2D> emissionmap;
	std::shared_ptr<image::texture::Texture_2D> mask;

	for (auto n = light_value.MemberBegin(); n != light_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("emission" == node_name) {
			emission = json::read_float3(node_value);
		} else if ("emission_factor" == node_name) {
			emission_factor = json::read_float(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				std::string filename = json::read_string(*tn, "file", "");
				std::string usage    = json::read_string(*tn, "usage", "Color");

				if (filename.empty()) {
					continue;
				}

				if ("Emission" == usage) {
					emissionmap = texture_cache_.load(filename);
				} else if ("Mask" == usage) {
					mask = texture_cache_.load(filename,
											   static_cast<uint32_t>(
												   image::texture::Provider::Flags::Use_as_mask));
				}
			}
		}
	}

	if (emissionmap) {
		return std::make_shared<light::Emissionmap>(light_cache_, mask, emissionmap, emission_factor);
	}

	return std::make_shared<light::Constant>(light_cache_, mask, emission);
}

std::shared_ptr<IMaterial> Provider::load_matte(const rapidjson::Value& matte_value) {
	math::float3 color(0.75f, 0.75f, 0.75f);
	float roughness = 1.f;
	std::shared_ptr<image::texture::Texture_2D> colormap;
	std::shared_ptr<image::texture::Texture_2D> normalmap;
	std::shared_ptr<image::texture::Texture_2D> mask;
	bool two_sided = false;

	for (auto n = matte_value.MemberBegin(); n != matte_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("color" == node_name) {
			color = json::read_float3(node_value);
		} else if ("roughness" == node_name) {
			roughness = json::read_float(node_value);
		} else if ("two_sided" == node_name) {
			two_sided = json::read_bool(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				std::string filename = json::read_string(*tn, "file", "");
				std::string usage    = json::read_string(*tn, "usage", "Color");

				if (filename.empty()) {
					continue;
				}

				if ("Color" == usage) {
					colormap = texture_cache_.load(filename);
				} else if ("Normal" == usage) {
					normalmap = texture_cache_.load(filename,
													static_cast<uint32_t>(
														image::texture::Provider::Flags::Use_as_normal));
				} else if ("Mask" == usage) {
					mask = texture_cache_.load(filename,
											   static_cast<uint32_t>(
												   image::texture::Provider::Flags::Use_as_mask));
				}
			}
		}
	}

	if (normalmap) {
		return std::make_shared<matte::Normalmap>(matte_cache_, mask, two_sided, color, normalmap, roughness);
	}

	return std::make_shared<matte::Constant>(matte_cache_, mask, two_sided, color, roughness);
}

std::shared_ptr<IMaterial> Provider::load_substitute(const rapidjson::Value& substitute_value) {
	math::float3 color(0.75f, 0.75f, 0.75f);
	float roughness = 0.9f;
	float metallic = 0.f;
	float emission_factor = 1.f;
	std::shared_ptr<image::texture::Texture_2D> colormap;
	std::shared_ptr<image::texture::Texture_2D> normalmap;
	std::shared_ptr<image::texture::Texture_2D> surfacemap;
	std::shared_ptr<image::texture::Texture_2D> emissionmap;
	std::shared_ptr<image::texture::Texture_2D> mask;
	bool two_sided = false;

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
		} else if ("two_sided" == node_name) {
			two_sided = json::read_bool(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				std::string filename = json::read_string(*tn, "file", "");
				std::string usage    = json::read_string(*tn, "usage", "Color");

				if (filename.empty()) {
					continue;
				}

				if ("Color" == usage) {
					colormap = texture_cache_.load(filename);
				} else if ("Normal" == usage) {
					normalmap = texture_cache_.load(filename,
													static_cast<uint32_t>(
														image::texture::Provider::Flags::Use_as_normal));
				} else if ("Surface" == usage) {
					surfacemap = texture_cache_.load(filename,
													 static_cast<uint32_t>(
														 image::texture::Provider::Flags::Use_as_surface));
				} else if ("Emission" == usage) {
					emissionmap = texture_cache_.load(filename);
				} else if ("Mask" == usage) {
					mask = texture_cache_.load(filename,
											   static_cast<uint32_t>(
												   image::texture::Provider::Flags::Use_as_mask));
				}
			}
		}
	}

	if (colormap) {
		if (normalmap) {
			if (surfacemap) {
				if (emissionmap) {
					return std::make_shared<substitute::Colormap_normalmap_surfacemap_emissionmap>(
								substitute_cache_, mask, two_sided,
								colormap, normalmap, surfacemap, emissionmap, emission_factor);
				} else {
					return std::make_shared<substitute::Colormap_normalmap_surfacemap>(
								substitute_cache_, mask, two_sided, colormap, normalmap, surfacemap);
				}
			} else {
				return std::make_shared<substitute::Colormap_normalmap>(
							substitute_cache_, mask, two_sided, colormap, normalmap, roughness, metallic);
			}
		} else {
			if (surfacemap) {
				return std::make_shared<substitute::Colormap_surfacemap>(
							substitute_cache_, mask, two_sided, colormap, surfacemap);
			} else {
				return std::make_shared<substitute::Colormap>(
							substitute_cache_, mask, two_sided, colormap, roughness, metallic);
			}
		}
	} else {
		if (normalmap) {
			return std::make_shared<substitute::Normalmap>(
						substitute_cache_, mask, two_sided, color, normalmap, roughness, metallic);
		}
	}

	return std::make_shared<substitute::Constant>(
				substitute_cache_, mask, two_sided, color, roughness, metallic);
}

}}

#include "material_provider.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_cache.inl"
#include "material_sample_cache.inl"
#include "image/texture/texture_2d_provider.hpp"
#include "glass/glass_material.hpp"
#include "light/light_constant.hpp"
#include "light/light_emissionmap.hpp"
#include "metal/metal_material.hpp"
#include "substitute/substitute_material.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include <iostream>

namespace scene { namespace material {

Provider::Provider(file::System& file_system,
				   resource::Cache<image::texture::Texture_2D>& texture_cache,
				   uint32_t num_workers) :
	resource::Provider<IMaterial>(file_system),
	texture_cache_(texture_cache),
	glass_cache_(num_workers),
	light_cache_(num_workers),
	metal_iso_cache_(num_workers),
	metal_aniso_cache_(num_workers),
	substitute_cache_(num_workers) {
	auto material = std::make_shared<substitute::Material>(substitute_cache_, nullptr, false);
	material->set_color(math::float3(1.f, 0.f, 0.f)),
	material->set_roughness(1.f);
	material->set_metallic(0.f);
	fallback_material_ = material;
}

std::shared_ptr<IMaterial> Provider::load(const std::string& filename, uint32_t /*flags*/) {
	auto stream_pointer = file_system_.read_stream(filename);

	auto root = json::parse(*stream_pointer);

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
		} else if ("Metal" == node_name) {
			return load_metal(node_value);
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
	std::shared_ptr<image::texture::Texture_2D> normal_map;
	math::float3 color(1.f, 1.f, 1.f);
	float attenuation_distance = 1.f;
	float ior = 1.5f;

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
					normal_map = texture_cache_.load(filename, static_cast<uint32_t>(
														 image::texture::Provider::Flags::Use_as_normal));
				}
			}
		}
	}

	auto material = std::make_shared<glass::Glass>(glass_cache_, nullptr);

	material->set_normal_map(normal_map);

	material->set_color(color);
	material->set_attenuation_distance(attenuation_distance);
	material->set_ior(ior);

	return material;
}

std::shared_ptr<IMaterial> Provider::load_light(const rapidjson::Value& light_value) {
	math::float3 emission(10.f, 10.f, 10.f);
	float emission_factor = 1.f;

	std::shared_ptr<image::texture::Texture_2D> emissionmap;
	std::shared_ptr<image::texture::Texture_2D> mask;
	bool two_sided = false;

	for (auto n = light_value.MemberBegin(); n != light_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("emission" == node_name) {
			emission = json::read_float3(node_value);
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
		return std::make_shared<light::Emissionmap>(light_cache_, mask, two_sided, emissionmap, emission_factor);
	}

	return std::make_shared<light::Constant>(light_cache_, mask, two_sided, emission);
}

std::shared_ptr<IMaterial> Provider::load_metal(const rapidjson::Value& substitute_value) {
	std::shared_ptr<image::texture::Texture_2D> normal_map;
//	std::shared_ptr<image::texture::Texture_2D> surface_map;
	std::shared_ptr<image::texture::Texture_2D> direction_map;
	std::shared_ptr<image::texture::Texture_2D> mask;
	bool two_sided = false;
	math::float3 ior(1.f, 1.f, 1.f);
	math::float3 absorption(0.75f, 0.75f, 0.75f);
	math::float2 anisotropy(0.f, 0.f);
	float roughness = 0.9f;
	math::float2 roughness_aniso(0.f, 0.f);

	for (auto n = substitute_value.MemberBegin(); n != substitute_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("ior" == node_name) {
			ior = json::read_float3(node_value);
		} else if ("absorption" == node_name) {
			absorption = json::read_float3(node_value);
		} else if ("roughness" == node_name) {
			if (node_value.IsArray()) {
				roughness_aniso = json::read_float2(node_value);
			} else {
				roughness = json::read_float(node_value);
			}
		} else if ("two_sided" == node_name) {
			two_sided = json::read_bool(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				std::string filename = json::read_string(*tn, "file", "");
				std::string usage    = json::read_string(*tn, "usage", "Color");

				if (filename.empty()) {
					continue;
				}

				if ("Normal" == usage) {
					normal_map = texture_cache_.load(filename,
													 static_cast<uint32_t>(
														image::texture::Provider::Flags::Use_as_normal));
			/*	} else if ("Surface" == usage) {
					surface_map = texture_cache_.load(filename,
													  static_cast<uint32_t>(
														 image::texture::Provider::Flags::Use_as_surface));*/
				} else if ("Direction" == usage) {
					direction_map = texture_cache_.load(filename,
														static_cast<uint32_t>(
														image::texture::Provider::Flags::Use_as_direction));
				} else if ("Mask" == usage) {
					mask = texture_cache_.load(filename,
											   static_cast<uint32_t>(
												   image::texture::Provider::Flags::Use_as_mask));
				}
			}
		}
	}

	if (roughness_aniso.x > 0.f && roughness_aniso.y > 0.f) {
		auto material = std::make_shared<metal::Material_aniso>(metal_aniso_cache_, mask, two_sided);

		material->set_normal_map(normal_map);
	//	material->set_surface_map(surface_map);
		material->set_direction_map(direction_map);

		material->set_ior(ior);
		material->set_absorption(absorption);
		material->set_roughness(roughness_aniso);

		return material;
	} else {
		auto material = std::make_shared<metal::Material_iso>(metal_iso_cache_, mask, two_sided);

		material->set_normal_map(normal_map);
	//	material->set_surface_map(surface_map);

		material->set_ior(ior);
		material->set_absorption(absorption);
		material->set_roughness(roughness);

		return material;
	}
}

std::shared_ptr<IMaterial> Provider::load_substitute(const rapidjson::Value& substitute_value) {
	std::shared_ptr<image::texture::Texture_2D> color_map;
	std::shared_ptr<image::texture::Texture_2D> normal_map;
	std::shared_ptr<image::texture::Texture_2D> surface_map;
	std::shared_ptr<image::texture::Texture_2D> emission_map;
	std::shared_ptr<image::texture::Texture_2D> mask;
	bool two_sided = false;
	math::float3 color(0.75f, 0.75f, 0.75f);
	float roughness = 0.9f;
	float metallic = 0.f;
	float emission_factor = 1.f;
	float thickness = 0.f;
	float attenuation_distance = 0.f;

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
		} else if ("thickness" == node_name) {
			thickness = json::read_float(node_value);
		} else if ("attenuation_distance" == node_name) {
			attenuation_distance = json::read_float(node_value);
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
					color_map = texture_cache_.load(filename);
				} else if ("Normal" == usage) {
					normal_map = texture_cache_.load(filename,
													 static_cast<uint32_t>(
														image::texture::Provider::Flags::Use_as_normal));
				} else if ("Surface" == usage) {
					surface_map = texture_cache_.load(filename,
													  static_cast<uint32_t>(
														 image::texture::Provider::Flags::Use_as_surface));
				} else if ("Emission" == usage) {
					emission_map = texture_cache_.load(filename);
				} else if ("Mask" == usage) {
					mask = texture_cache_.load(filename,
												static_cast<uint32_t>(
												   image::texture::Provider::Flags::Use_as_mask));
				}
			}
		}
	}

	auto material = std::make_shared<substitute::Material>(substitute_cache_, mask, two_sided);

	material->set_color_map(color_map);
	material->set_normal_map(normal_map);
	material->set_surface_map(surface_map);
	material->set_emission_map(emission_map);

	material->set_color(color);
	material->set_roughness(roughness);
	material->set_metallic(metallic);
	material->set_emission_factor(emission_factor);
	material->set_thickness(thickness);
	material->set_attenuation_distance(attenuation_distance);

	return material;
}

}}

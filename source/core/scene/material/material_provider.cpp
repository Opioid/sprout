#include "material_provider.hpp"
#include "logging/logging.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_manager.inl"
#include "material_sample_cache.inl"
#include "image/texture/texture_2d_adapter.inl"
#include "image/texture/texture_2d_provider.hpp"
#include "cloth/cloth_material.hpp"
#include "cloth/cloth_sample.hpp"
#include "display/display_material.hpp"
#include "display/display_material_animated.hpp"
#include "display/display_sample.hpp"
#include "glass/glass_material.hpp"
#include "glass/glass_sample.hpp"
#include "glass/glass_rough_material.hpp"
#include "glass/glass_rough_sample.hpp"
#include "light/light_constant.hpp"
#include "light/light_emissionmap.hpp"
#include "light/light_emissionmap_animated.hpp"
#include "light/light_material_sample.hpp"
#include "matte/matte_material.hpp"
#include "matte/matte_sample.hpp"
#include "metal/metal_material.hpp"
#include "metal/metal_presets.hpp"
#include "metal/metal_sample.hpp"
#include "metallic_paint/metallic_paint_material.hpp"
#include "metallic_paint/metallic_paint_sample.hpp"
#include "sky/sky_material_overcast.hpp"
#include "substitute/substitute_base_material.inl"
#include "substitute/substitute_coating_material.inl"
#include "substitute/substitute_coating_sample.inl"
#include "substitute/substitute_material.hpp"
#include "substitute/substitute_sample.hpp"
#include "substitute/substitute_translucent_material.hpp"
#include "substitute/substitute_translucent_sample.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/memory/variant_map.inl"
#include "base/spectrum/blackbody.hpp"
#include "base/spectrum/rgb.inl"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"

namespace scene { namespace material {

Provider::Provider(uint32_t num_threads) :
	resource::Provider<Material>("Material"),
	cloth_cache_(num_threads),
	display_cache_(num_threads),
	glass_cache_(num_threads),
	glass_rough_cache_(num_threads),
	light_cache_(num_threads),
	matte_cache_(num_threads),
	metal_iso_cache_(num_threads),
	metal_aniso_cache_(num_threads),
	metallic_paint_cache_(num_threads),
	substitute_cache_(num_threads),
	substitute_clearcoat_cache_(num_threads),
	substitute_thinfilm_cache_(num_threads),
	substitute_translucent_cache_(num_threads) {
	auto material = std::make_shared<substitute::Material>(
				substitute_cache_, Sampler_settings(Sampler_settings::Filter::Linear), false);
	material->set_color(float3(1.f, 0.f, 0.f)),
	material->set_ior(1.45f),
	material->set_roughness(1.f);
	material->set_metallic(0.f);
	fallback_material_ = material;
}

Provider::~Provider() {}

std::shared_ptr<Material> Provider::load(const std::string& filename,
										 const memory::Variant_map& /*options*/,
										 resource::Manager& manager) {
	std::string resolved_name;
	auto stream_pointer = manager.file_system().read_stream(filename, resolved_name);

	auto root = json::parse(*stream_pointer);

	return load(*root, string::parent_directory(resolved_name), manager);
}

std::shared_ptr<Material> Provider::load(const void* data,
										 const std::string& mount_folder,
										 const memory::Variant_map& /*options*/,
										 resource::Manager& manager) {
	const json::Value* value = reinterpret_cast<const json::Value*>(data);

	return load(*value, mount_folder, manager);
}

std::shared_ptr<Material> Provider::load(const json::Value& value,
										 const std::string& mount_folder,
										 resource::Manager& manager) {
	const json::Value::ConstMemberIterator rendering_node = value.FindMember("rendering");
	if (value.MemberEnd() == rendering_node) {
		throw std::runtime_error("Material has no render node");
	}

	manager.file_system().push_mount(mount_folder);

	std::shared_ptr<Material> material;

	const json::Value& rendering_value = rendering_node->value;

	for (auto n = rendering_value.MemberBegin(); n != rendering_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("Cloth" == node_name) {
			material = load_cloth(node_value, manager);
		} else if ("Display" == node_name) {
			material = load_display(node_value, manager);
		} else if ("Glass" == node_name) {
			material = load_glass(node_value, manager);
		} else if ("Light" == node_name) {
			material = load_light(node_value, manager);
		} else if ("Matte" == node_name) {
			material = load_matte(node_value, manager);
		} else if ("Metal" == node_name) {
			material = load_metal(node_value, manager);
		} else if ("Metallic_paint" == node_name) {
			material = load_metallic_paint(node_value, manager);
		} else if ("Sky" == node_name) {
			material = load_sky(node_value, manager);
		} else if ("Substitute" == node_name) {
			material = load_substitute(node_value, manager);
		}
	}

	manager.file_system().pop_mount();

	if (!material) {
		throw std::runtime_error("Material is of unknown type");
	}

	return material;
}

std::shared_ptr<Material> Provider::fallback_material() const {
	return fallback_material_;
}

Generic_sample_cache<light::Sample>& Provider::light_cache() {
	return light_cache_;
}

std::shared_ptr<Material> Provider::load_cloth(const json::Value& cloth_value,
											   resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	Adapter_2D color_map;
	Adapter_2D normal_map;
	Adapter_2D mask;
	bool two_sided = false;
	float3 color(0.75f, 0.75f, 0.75f);

	for (auto n = cloth_value.MemberBegin(); n != cloth_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("color" == node_name) {
			color = json::read_float3(node_value);
		} else if ("two_sided" == node_name) {
			two_sided = json::read_bool(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				if ("Color" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Color);
					color_map = create_texture(texture_description, options, manager);
				} else if ("Normal" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Normal);
					normal_map = create_texture(texture_description, options, manager);
				} else if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	auto material = std::make_shared<cloth::Material>(cloth_cache_, sampler_settings, two_sided);

	material->set_mask(mask);
	material->set_color_map(color_map);
	material->set_normal_map(normal_map);

	material->set_color(color);

	return material;
}

std::shared_ptr<Material> Provider::load_display(const json::Value& display_value,
												 resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	Adapter_2D mask;
	Adapter_2D emission_map;
	bool two_sided = false;

	float3 radiance(10.f, 10.f, 10.f);
	float emission_factor = 1.f;
	float roughness = 1.f;
	float ior = 1.5;
	float animation_duration = 0.f;

	for (auto n = display_value.MemberBegin(); n != display_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("radiance" == node_name) {
			radiance = json::read_float3(node_value);
		} else if ("emission_factor" == node_name) {
			emission_factor = json::read_float(node_value);
		} else if ("roughness" == node_name) {
			roughness = json::read_float(node_value);
		} else if ("ior" == node_name) {
			ior = json::read_float(node_value);
		} else if ("two_sided" == node_name) {
			two_sided = json::read_bool(node_value);
		} else if ("animation_duration" == node_name) {
			animation_duration = json::read_float(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;

				if (texture_description.num_elements > 1) {
					options.set("num_elements", texture_description.num_elements);
				}

				if ("Emission" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Color);
					emission_map = create_texture(texture_description, options, manager);
				} else if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	if (animation_duration > 0.f) {
		auto material = std::make_shared<display::Material_animated>(
					display_cache_, sampler_settings, two_sided, emission_map, animation_duration);
		material->set_mask(mask);
		material->set_emission_factor(emission_factor);
		material->set_roughness(roughness);
		material->set_ior(ior);
		return material;
	} else {
		auto material = std::make_shared<display::Material>(
					display_cache_, sampler_settings, two_sided);

		material->set_mask(mask);
		material->set_emission_map(emission_map);
		material->set_emission(radiance);
		material->set_emission_factor(emission_factor);
		material->set_roughness(roughness);
		material->set_ior(ior);
		return material;
	}
}

std::shared_ptr<Material> Provider::load_glass(const json::Value& glass_value,
											   resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	Adapter_2D normal_map;
	Adapter_2D roughness_map;
	float3 refraction_color(1.f, 1.f, 1.f);
	float3 absorbtion_color(1.f, 1.f, 1.f);
	float attenuation_distance = 1.f;
	float ior = 1.5f;
	float roughness = 0.f;

	for (auto n = glass_value.MemberBegin(); n != glass_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("color" == node_name) {
			refraction_color = json::read_float3(node_value);
			absorbtion_color = refraction_color;
		} else if ("refraction_color" == node_name) {
			refraction_color = json::read_float3(node_value);
		} else if ("absorbtion_color" == node_name) {
			absorbtion_color = json::read_float3(node_value);
		} else if ("attenuation_distance" == node_name) {
			attenuation_distance = json::read_float(node_value);
		} else if ("ior" == node_name) {
			ior = json::read_float(node_value);
		} else if ("roughness" == node_name) {
			roughness = json::read_float(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				if ("Normal" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Normal);
					normal_map = create_texture(texture_description, options, manager);
				} else if ("Roughness" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Roughness);
					roughness_map = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	if (roughness > 0.f || roughness_map.is_valid()) {
		auto material = std::make_shared<glass::Glass_rough>(glass_rough_cache_,
															 sampler_settings);
		material->set_normal_map(normal_map);
		material->set_roughness_map(roughness_map);
		material->set_refraction_color(refraction_color);
		material->set_absorbtion_color(absorbtion_color);
		material->set_attenuation_distance(attenuation_distance);
		material->set_ior(ior);
		material->set_roughness(roughness);
		return material;
	} else {
		auto material = std::make_shared<glass::Glass>(glass_cache_, sampler_settings);

		material->set_normal_map(normal_map);
		material->set_refraction_color(refraction_color);
		material->set_absorbtion_color(absorbtion_color);
		material->set_attenuation_distance(attenuation_distance);
		material->set_ior(ior);
		return material;
	}
}

std::shared_ptr<Material> Provider::load_light(const json::Value& light_value,
											   resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	std::string quantity;
	float3 color(1.f, 1.f, 1.f);
	float value = 1.f;

	float3 radiance(10.f, 10.f, 10.f);
	float emission_factor = 1.f;
	float animation_duration = 0.f;

	Adapter_2D emission_map;
	Adapter_2D mask;
	bool two_sided = false;

	for (auto n = light_value.MemberBegin(); n != light_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("emission" == node_name) {
			radiance = json::read_float3(node_value);
		} else if ("emittance" == node_name) {
			quantity = json::read_string(node_value, "quantity");

			auto s = node_value.FindMember("spectrum");
			if (node_value.MemberEnd() != s) {
				color = read_spectrum(s->value);
			}

			value = json::read_float(node_value, "value", value);
		} else if ("emission_factor" == node_name) {
			emission_factor = json::read_float(node_value);
		} else if ("two_sided" == node_name) {
			two_sided = json::read_bool(node_value);
		} else if ("animation_duration" == node_name) {
			animation_duration = json::read_float(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;

				if (texture_description.num_elements > 1) {
					options.set("num_elements", texture_description.num_elements);
				}

				if ("Emission" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Color);
					emission_map = create_texture(texture_description, options, manager);
				} else if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	if (emission_map.is_valid()) {
		if (animation_duration > 0.f) {
			auto material = std::make_shared<light::Emissionmap_animated>(
						light_cache_, sampler_settings, two_sided,
						emission_map, emission_factor, animation_duration);

			material->set_mask(mask);
			return material;
		} else {
			auto material = std::make_shared<light::Emissionmap>(
						light_cache_, sampler_settings, two_sided);

			material->set_mask(mask);
			material->set_emission_map(emission_map);
			material->set_emission_factor(emission_factor);
			return material;
		}
	}

	auto material = std::make_shared<light::Constant>(light_cache_, sampler_settings, two_sided);

	material->set_mask(mask);

	if ("Flux" == quantity) {
		material->emittance().set_flux(color, value);
	} else if ("Intensity" == quantity) {
		material->emittance().set_intensity(color, value);
	} else if ("Exitance" == quantity) {
		material->emittance().set_exitance(color, value);
	} else if ("Luminance" == quantity) {
		material->emittance().set_luminance(color, value);
	} else if ("Radiance" == quantity) {
		material->emittance().set_radiance(value * color);
	} else {
		material->emittance().set_radiance(radiance);
	}

	return material;
}

std::shared_ptr<Material> Provider::load_matte(const json::Value& substitute_value,
											   resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

//	Texture_2D_ptr normal_map;
	Adapter_2D mask;
	bool two_sided = false;
	float3 color(0.6f, 0.6f, 0.6f);

	for (auto n = substitute_value.MemberBegin(); n != substitute_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("color" == node_name) {
			color = json::read_float3(node_value);
		} else if ("two_sided" == node_name) {
			two_sided = json::read_bool(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				/*if ("Normal" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Normal);
					normal_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				} else*/ if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	auto material = std::make_shared<matte::Material>(matte_cache_, sampler_settings, two_sided);

	material->set_mask(mask);
//	material->set_normal_map(normal_map);

	material->set_color(color);

	return material;
}

std::shared_ptr<Material> Provider::load_metal(const json::Value& substitute_value,
											   resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	Adapter_2D normal_map;
//	Texture_2D_ptr surface_map;
	Adapter_2D direction_map;
	Adapter_2D mask;
	bool two_sided = false;
	float3 ior(1.f, 1.f, 1.f);
	float3 absorption(0.75f, 0.75f, 0.75f);
	float roughness = 0.9f;
	float2 roughness_aniso(0.f, 0.f);

	for (auto n = substitute_value.MemberBegin(); n != substitute_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("ior" == node_name) {
			ior = json::read_float3(node_value);
		} else if ("absorption" == node_name) {
			absorption = json::read_float3(node_value);
		} else if ("preset" == node_name) {
			metal::ior_and_absorption(node_value.GetString(), ior, absorption);
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
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				if ("Normal" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Normal);
					normal_map = create_texture(texture_description, options, manager);
			/*	} else if ("Surface" == usage) {
					surface_map = texture_cache_.load(filename,
													  static_cast<uint32_t>(
														 image::texture::Provider::Flags::Use_as_surface));*/
				} else if ("Anisotropy" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Anisotropy);
					direction_map = create_texture(texture_description, options, manager);
				} else if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	if (roughness_aniso.x > 0.f && roughness_aniso.y > 0.f) {
		auto material = std::make_shared<metal::Material_anisotropic>(
					metal_aniso_cache_, sampler_settings, two_sided);

		material->set_mask(mask);
		material->set_normal_map(normal_map);
	//	material->set_surface_map(surface_map);
		material->set_direction_map(direction_map);

		material->set_ior(ior);
		material->set_absorption(absorption);
		material->set_roughness(roughness_aniso);

		return material;
	} else {
		auto material = std::make_shared<metal::Material_isotropic>(
					metal_iso_cache_, sampler_settings, two_sided);

		material->set_mask(mask);
		material->set_normal_map(normal_map);
	//	material->set_surface_map(surface_map);

		material->set_ior(ior);
		material->set_absorption(absorption);
		material->set_roughness(roughness);

		return material;
	}
}

std::shared_ptr<Material> Provider::load_metallic_paint(const json::Value& substitute_value,
														resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	Adapter_2D mask;
	Adapter_2D flakes_normal_map;
	Adapter_2D flakes_mask;
	bool two_sided = false;
	float3 color_a(1.f, 0.f, 0.f);
	float3 color_b(0.f, 0.f, 1.f);
	float roughness = 0.575f;
	float flakes_size = 0.1f;
	float flakes_density = 0.2f;
	float3 flakes_ior(1.f, 1.f, 1.f);
	float3 flakes_absorption(0.75f, 0.75f, 0.75f);
	float flakes_roughness = 0.3f;
	float2 flakes_scale(1.f, 1.f);
	Coating_description coating;
	coating.ior = 1.5f;

	for (auto n = substitute_value.MemberBegin(); n != substitute_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("color_a" == node_name) {
			color_a = json::read_float3(node_value);
		} else if ("color_b" == node_name) {
			color_b = json::read_float3(node_value);
		} else if ("roughness" == node_name) {
			roughness  = json::read_float(node_value);
		} else if ("two_sided" == node_name) {
			two_sided = json::read_bool(node_value);
		} else if ("flakes" == node_name) {
			std::string flakes_preset = json::read_string(node_value, "preset");

			if (flakes_preset.empty()) {
				flakes_ior = json::read_float3(node_value, "ior", flakes_ior);
				flakes_absorption = json::read_float3(node_value, "absorption", flakes_absorption);
			} else {
				metal::ior_and_absorption(flakes_preset, flakes_ior, flakes_absorption);
			}

			flakes_size = json::read_float(node_value, "size", flakes_size);
			flakes_density = json::read_float(node_value, "density", flakes_density);
			flakes_roughness = json::read_float(node_value, "roughness", flakes_roughness);
			flakes_scale = json::read_float2(node_value, "scale", flakes_scale);
		} else if ("coating" == node_name) {
			read_coating_description(node_value, coating);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	Texture_description texture_description;
	texture_description.scale = flakes_scale;

	memory::Variant_map options;
	options.set("size", flakes_size);
	options.set("density", flakes_density);

	texture_description.filename = "proc:flakes";
	options.set("usage", image::texture::Provider::Usage::Normal);
	flakes_normal_map = create_texture(texture_description, options, manager);

	texture_description.filename = "proc:flakes_mask";
	options.set("usage", image::texture::Provider::Usage::Mask);
	flakes_mask = create_texture(texture_description, options, manager);

	auto material = std::make_shared<metallic_paint::Material>(metallic_paint_cache_,
															   sampler_settings,
															   two_sided);

	material->set_mask(mask);

	material->set_color(color_a, color_b);
	material->set_roughness(roughness);

	material->set_flakes_mask(flakes_mask);
	material->set_flakes_normal_map(flakes_normal_map);
	material->set_flakes_ior(flakes_ior);
	material->set_flakes_absorption(flakes_absorption);
	material->set_flakes_roughness(flakes_roughness);

	material->set_coating_weight(coating.weight);
	material->set_coating_color(coating.color);
	material->set_clearcoat(coating.ior, coating.roughness);

	return material;
}

std::shared_ptr<Material> Provider::load_sky(const json::Value& sky_value,
											 resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	Adapter_2D mask;

	bool two_sided = false;

	float3 radiance(0.6f, 0.6f, 0.6f);

	for (auto n = sky_value.MemberBegin(); n != sky_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("radiance" == node_name) {
			radiance = json::read_float3(node_value);
		} else if ("two_sided" == node_name) {
			two_sided = json::read_bool(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	auto material = std::make_shared<sky::Material_overcast>(
				light_cache_, sampler_settings, two_sided);

	material->set_mask(mask);
	material->set_emission(radiance);

	return material;
}

std::shared_ptr<Material> Provider::load_substitute(const json::Value& substitute_value,
													resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	Adapter_2D color_map;
	Adapter_2D normal_map;
	Adapter_2D surface_map;
	Adapter_2D emission_map;
	Adapter_2D mask;
	bool two_sided = false;
	float3 color(0.4f, 0.4f, 0.4f);
	float roughness = 0.9f;
	float metallic = 0.f;
	float ior = 1.46f;
	float emission_factor = 1.f;
	float thickness = 0.f;
	float attenuation_distance = 0.f;
	Coating_description coating;

	for (auto n = substitute_value.MemberBegin(); n != substitute_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("color" == node_name) {
			color = json::read_float3(node_value);
		} else if ("ior" == node_name) {
			ior = json::read_float(node_value);
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
		} else if ("coating" == node_name) {
			read_coating_description(node_value, coating);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				if ("Color" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Color);
					color_map = create_texture(texture_description, options, manager);
				} else if ("Normal" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Normal);
					normal_map = create_texture(texture_description, options, manager);
				} else if ("Surface" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Surface);
					surface_map = create_texture(texture_description, options, manager);
				} else if ("Emission" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Color);
					emission_map = create_texture(texture_description, options, manager);
				} else if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	if (thickness > 0.f) {
		auto material = std::make_shared<substitute::Material_translucent>(
					substitute_translucent_cache_, sampler_settings, two_sided);

		material->set_mask(mask);
		material->set_color_map(color_map);
		material->set_normal_map(normal_map);
		material->set_surface_map(surface_map);
		material->set_emission_map(emission_map);

		material->set_color(color);
		material->set_ior(ior);
		material->set_roughness(roughness);
		material->set_metallic(metallic);
		material->set_emission_factor(emission_factor);
		material->set_thickness(thickness);
		material->set_attenuation_distance(attenuation_distance);

		return material;
	} else if (coating.ior > 1.f) {

		Adapter_2D coating_weight_map;
		Adapter_2D coating_normal_map;

		if (!coating.weight_map_description.filename.empty()) {
			memory::Variant_map options;
			options.set("usage", image::texture::Provider::Usage::Mask);
			coating_weight_map = create_texture(coating.weight_map_description, options, manager);
		}

		if (!coating.normal_map_description.filename.empty()) {
			memory::Variant_map options;
			options.set("usage", image::texture::Provider::Usage::Normal);
			coating_normal_map = create_texture(coating.normal_map_description, options, manager);
		}

		if (coating.thickness > 0.f) {
			auto material = std::make_shared<substitute::Material_thinfilm>(
						substitute_thinfilm_cache_, sampler_settings, two_sided);

			material->set_mask(mask);
			material->set_color_map(color_map);
			material->set_normal_map(normal_map);
			material->set_surface_map(surface_map);
			material->set_emission_map(emission_map);

			material->set_color(color);
			material->set_ior(ior);
			material->set_roughness(roughness);
			material->set_metallic(metallic);
			material->set_emission_factor(emission_factor);

			material->set_coating_normal_map(coating_normal_map);
			material->set_coating_weight_map(coating_weight_map);
			material->set_coating_weight(coating.weight);
			material->set_coating_color(coating.color);
			material->set_thinfilm(coating.ior, coating.roughness,
								   coating.thickness);

			return material;
		} else {
			auto material = std::make_shared<substitute::Material_clearcoat>(
						substitute_clearcoat_cache_, sampler_settings, two_sided);

			material->set_mask(mask);
			material->set_color_map(color_map);
			material->set_normal_map(normal_map);
			material->set_surface_map(surface_map);
			material->set_emission_map(emission_map);

			material->set_color(color);
			material->set_ior(ior/*, clearcoat.ior*/);
			material->set_roughness(roughness);
			material->set_metallic(metallic);
			material->set_emission_factor(emission_factor);

			material->set_coating_normal_map(coating_normal_map);
			material->set_coating_weight_map(coating_weight_map);
			material->set_coating_weight(coating.weight);
			material->set_coating_color(coating.color);
			material->set_clearcoat(coating.ior, coating.roughness);

			return material;
		}
	}

	auto material = std::make_shared<substitute::Material>(
				substitute_cache_, sampler_settings, two_sided);

	material->set_mask(mask);
	material->set_color_map(color_map);
	material->set_normal_map(normal_map);
	material->set_surface_map(surface_map);
	material->set_emission_map(emission_map);

	material->set_color(color);
	material->set_ior(ior);
	material->set_roughness(roughness);
	material->set_metallic(metallic);
	material->set_emission_factor(emission_factor);

	return material;
}

void Provider::read_sampler_settings(const json::Value& sampler_value,
									 Sampler_settings& settings) {
	for (auto n = sampler_value.MemberBegin(); n != sampler_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("filter" == node_name) {
			std::string filter = json::read_string(node_value);

			if ("Nearest" == filter) {
				settings.filter = Sampler_settings::Filter::Nearest;
			} else if ("Linear" == filter) {
				settings.filter = Sampler_settings::Filter::Linear;
			}
		}
	}
}

void Provider::read_texture_description(const json::Value& texture_value,
										Texture_description& description) {
	description.filename = "";
	description.usage = "Color";
	description.scale = float2(1.f, 1.f);
	description.num_elements = 1;

	for (auto n = texture_value.MemberBegin(); n != texture_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("file" == node_name) {
			description.filename = json::read_string(node_value);
		} else if ("usage" == node_name) {
			description.usage = json::read_string(node_value);
		} else if ("scale" == node_name) {
			description.scale = json::read_float2(node_value);
		} else if ("num_elements" == node_name) {
			description.num_elements = json::read_int(node_value);
		}
	}
}

Adapter_2D Provider::create_texture(const Texture_description& description,
									const memory::Variant_map& options,
									resource::Manager& manager) {
	return Adapter_2D(manager.load<image::texture::Texture_2D>(description.filename, options),
					  description.scale);
}

void Provider::read_coating_description(const json::Value& coating_value,
										Coating_description& description) {
	if (!coating_value.IsObject()) {
		return;
	}

	for (auto n = coating_value.MemberBegin(); n != coating_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("color" == node_name) {
			description.color = json::read_float3(node_value);
		} else if ("ior" == node_name) {
			description.ior = json::read_float(node_value);
		} else if ("roughness" == node_name) {
			description.roughness = json::read_float(node_value);
		} else if ("thickness" == node_name) {
			description.thickness = json::read_float(node_value);
		} else if ("weight" == node_name) {
			description.weight = json::read_float(node_value);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				if ("Normal" == texture_description.usage) {
					description.normal_map_description = texture_description;
				} else if ("Mask" == texture_description.usage) {
					description.weight_map_description = texture_description;
				}
			}
		}
	}
}

float3 Provider::read_spectrum(const json::Value& spectrum_value) {
	if (!spectrum_value.IsObject()) {
		return float3(0.f);
	}

	for (auto n = spectrum_value.MemberBegin(); n != spectrum_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("sRGB" == node_name) {
			float3 srgb = json::read_float3(node_value);
			return spectrum::sRGB_to_linear_RGB(srgb);
		} else if ("RGB" == node_name) {
			return json::read_float3(node_value);
		} else if ("temperature" == node_name) {
			float temperature = json::read_float(node_value);
			temperature = std::max(800.f, temperature);
			return spectrum::blackbody(temperature);
		}
	}

	return float3(0.f);
}

}}

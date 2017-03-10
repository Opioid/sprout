#include "material_provider.hpp"
#include "logging/logging.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_manager.inl"
#include "bssrdf.hpp"
#include "material_sample_cache.inl"
#include "image/texture/texture_adapter.inl"
#include "image/texture/texture_provider.hpp"
#include "cloth/cloth_material.hpp"
#include "cloth/cloth_sample.hpp"
#include "display/display_constant.hpp"
#include "display/display_emissionmap.hpp"
#include "display/display_emissionmap_animated.hpp"
#include "display/display_sample.hpp"
#include "glass/glass_material.hpp"
#include "glass/glass_sample.hpp"
#include "glass/glass_rough_material.hpp"
#include "glass/glass_rough_sample.hpp"
#include "glass/thinglass_material.hpp"
#include "glass/thinglass_sample.hpp"
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
#include "substitute/substitute_subsurface_material.hpp"
#include "substitute/substitute_subsurface_sample.hpp"
#include "substitute/substitute_translucent_material.hpp"
#include "substitute/substitute_translucent_sample.hpp"
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "base/memory/variant_map.inl"
#include "base/spectrum/blackbody.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"

namespace scene { namespace material {

Provider::Provider(uint32_t num_threads) :
	resource::Provider<Material>("Material"),
	sample_cache_(max_sample_size(), num_threads) {
	auto material = std::make_shared<substitute::Material>(
				sample_cache_, Sampler_settings(Sampler_settings::Filter::Linear), false);
	material->set_color(float3(1.f, 0.f, 0.f)),
	material->set_ior(1.45f),
	material->set_roughness(1.f);
	material->set_metallic(0.f);
	fallback_material_ = material;
}

Provider::~Provider() {}

Material_ptr Provider::load(const std::string& filename, const memory::Variant_map& /*options*/,
							resource::Manager& manager) {
	std::string resolved_name;
	auto stream_pointer = manager.file_system().read_stream(filename, resolved_name);

	auto root = json::parse(*stream_pointer);

	return load(*root, string::parent_directory(resolved_name), manager);
}

Material_ptr Provider::load(const void* data, const std::string& mount_folder,
							const memory::Variant_map& /*options*/, resource::Manager& manager) {
	const json::Value* value = reinterpret_cast<const json::Value*>(data);

	return load(*value, mount_folder, manager);
}

size_t Provider::num_bytes() const {
	return sizeof(*this) + sample_cache_.num_bytes();
}

Material_ptr Provider::fallback_material() const {
	return fallback_material_;
}

Sample_cache& Provider::sample_cache() {
	return sample_cache_;
}

Material_ptr Provider::load(const json::Value& value, const std::string& mount_folder,
							resource::Manager& manager) {
	const json::Value::ConstMemberIterator rendering_node = value.FindMember("rendering");
	if (value.MemberEnd() == rendering_node) {
		throw std::runtime_error("Material has no render node");
	}

	manager.file_system().push_mount(mount_folder);

	std::shared_ptr<Material> material;

	const json::Value& rendering_value = rendering_node->value;

	for (auto& n : rendering_value.GetObject()) {
		if ("Cloth" == n.name) {
			material = load_cloth(n.value, manager);
		} else if ("Display" == n.name) {
			material = load_display(n.value, manager);
		} else if ("Glass" == n.name) {
			material = load_glass(n.value, manager);
		} else if ("Light" == n.name) {
			material = load_light(n.value, manager);
		} else if ("Matte" == n.name) {
			material = load_matte(n.value, manager);
		} else if ("Metal" == n.name) {
			material = load_metal(n.value, manager);
		} else if ("Metallic_paint" == n.name) {
			material = load_metallic_paint(n.value, manager);
		} else if ("Sky" == n.name) {
			material = load_sky(n.value, manager);
		} else if ("Substitute" == n.name) {
			material = load_substitute(n.value, manager);
		}
	}

	manager.file_system().pop_mount();

	if (!material) {
		throw std::runtime_error("Material is of unknown type");
	}

	return material;
}

Material_ptr Provider::load_cloth(const json::Value& cloth_value, resource::Manager& manager) {
	Sampler_settings sampler_settings;

	Texture_adapter color_map;
	Texture_adapter normal_map;
	Texture_adapter mask;
	bool two_sided = false;
	float3 color(0.75f, 0.75f, 0.75f);

	for (auto& n : cloth_value.GetObject()) {
		if ("color" == n.name) {
			color = json::read_float3(n.value);
		} else if ("two_sided" == n.name) {
			two_sided = json::read_bool(n.value);
		} else if ("textures" == n.name) {
			for (auto& tn : n.value.GetArray()) {
				Texture_description texture_description;
				read_texture_description(tn, texture_description);

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
		} else if ("sampler" == n.name) {
			read_sampler_settings(n.value, sampler_settings);
		}
	}

	auto material = std::make_shared<cloth::Material>(sample_cache_, sampler_settings, two_sided);

	material->set_mask(mask);
	material->set_color_map(color_map);
	material->set_normal_map(normal_map);

	material->set_color(color);

	return material;
}

Material_ptr Provider::load_display(const json::Value& display_value, resource::Manager& manager) {
	Sampler_settings sampler_settings;

	Texture_adapter mask;
	Texture_adapter emission_map;
	bool two_sided = false;

	float3 radiance(10.f, 10.f, 10.f);
	float emission_factor = 1.f;
	float roughness = 1.f;
	float ior = 1.5;
	float animation_duration = 0.f;

	for (auto& n : display_value.GetObject()) {
		if ("radiance" == n.name) {
			radiance = json::read_float3(n.value);
		} else if ("emission_factor" == n.name) {
			emission_factor = json::read_float(n.value);
		} else if ("roughness" == n.name) {
			roughness = json::read_float(n.value);
		} else if ("ior" == n.name) {
			ior = json::read_float(n.value);
		} else if ("two_sided" == n.name) {
			two_sided = json::read_bool(n.value);
		} else if ("animation_duration" == n.name) {
			animation_duration = json::read_float(n.value);
		} else if ("textures" == n.name) {
			for (auto& tn : n.value.GetArray()) {
				Texture_description texture_description;
				read_texture_description(tn, texture_description);

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
		} else if ("sampler" == n.name) {
			read_sampler_settings(n.value, sampler_settings);
		}
	}

	if (emission_map.is_valid()) {
		if (animation_duration > 0.f) {
			auto material = std::make_shared<display::Emissionmap_animated>(
						sample_cache_, sampler_settings, two_sided,
						emission_map, emission_factor, animation_duration);
			material->set_mask(mask);
			material->set_roughness(roughness);
			material->set_ior(ior);
			return material;
		} else {
			auto material = std::make_shared<display::Emissionmap>(sample_cache_, sampler_settings,
																   two_sided);
			material->set_mask(mask);
			material->set_emission_map(emission_map);
			material->set_emission_factor(emission_factor);
			material->set_roughness(roughness);
			material->set_ior(ior);
			return material;
		}
	}

	auto material = std::make_shared<display::Constant>(sample_cache_, sampler_settings,
														two_sided);
	material->set_mask(mask);
	material->set_emission(radiance);
	material->set_roughness(roughness);
	material->set_ior(ior);
	return material;
}

Material_ptr Provider::load_glass(const json::Value& glass_value, resource::Manager& manager) {
	Sampler_settings sampler_settings;

	Texture_adapter normal_map;
	Texture_adapter roughness_map;

	float3 refraction_color(1.f, 1.f, 1.f);
	float3 absorption_color(1.f, 1.f, 1.f);
	float attenuation_distance = 1.f;
	float ior = 1.5f;
	float roughness = 0.f;
	float thickness = 0.f;

	for (auto& n : glass_value.GetObject()) {
		if ("color" == n.name) {
			refraction_color = json::read_float3(n.value);
			absorption_color = refraction_color;
		} else if ("refraction_color" == n.name) {
			refraction_color = json::read_float3(n.value);
		} else if ("absorption_color" == n.name) {
			absorption_color = json::read_float3(n.value);
		} else if ("attenuation_distance" == n.name) {
			attenuation_distance = json::read_float(n.value);
		} else if ("ior" == n.name) {
			ior = json::read_float(n.value);
		} else if ("roughness" == n.name) {
			roughness = json::read_float(n.value);
		} else if ("thickness" == n.name) {
			thickness = json::read_float(n.value);
		} else if ("textures" == n.name) {
			for (auto& tn : n.value.GetArray()) {
				Texture_description texture_description;
				read_texture_description(tn, texture_description);

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
		} else if ("sampler" == n.name) {
			read_sampler_settings(n.value, sampler_settings);
		}
	}

	if (roughness > 0.f || roughness_map.is_valid()) {
		auto material = std::make_shared<glass::Glass_rough>(sample_cache_, sampler_settings);
		material->set_normal_map(normal_map);
		material->set_roughness_map(roughness_map);
		material->set_refraction_color(refraction_color);
		material->set_absorption_color(absorption_color);
		material->set_attenuation_distance(attenuation_distance);
		material->set_ior(ior);
		material->set_roughness(roughness);
		return material;
	} else {
		if (thickness > 0.f) {
			auto material = std::make_shared<glass::Thinglass>(sample_cache_, sampler_settings);
			material->set_normal_map(normal_map);
			material->set_refraction_color(refraction_color);
			material->set_absorption_color(absorption_color);
			material->set_attenuation_distance(attenuation_distance);
			material->set_ior(ior);
			material->set_thickness(thickness);
			return material;
		} else {
			auto material = std::make_shared<glass::Glass>(sample_cache_, sampler_settings);
			material->set_normal_map(normal_map);
			material->set_refraction_color(refraction_color);
			material->set_absorption_color(absorption_color);
			material->set_attenuation_distance(attenuation_distance);
			material->set_ior(ior);
			return material;
		}
	}
}

Material_ptr Provider::load_light(const json::Value& light_value, resource::Manager& manager) {
	Sampler_settings sampler_settings;

	std::string quantity;
	float3 color(1.f, 1.f, 1.f);
	float value = 1.f;

	float3 radiance(10.f, 10.f, 10.f);
	float emission_factor = 1.f;
	float animation_duration = 0.f;

	Texture_adapter emission_map;
	Texture_adapter mask;
	bool two_sided = false;

	for (auto& n : light_value.GetObject()) {
		if ("emission" == n.name) {
			radiance = json::read_float3(n.value);
		} else if ("emittance" == n.name) {
			quantity = json::read_string(n.value, "quantity");

			auto s = n.value.FindMember("spectrum");
			if (n.value.MemberEnd() != s) {
				color = read_spectrum(s->value);
			}

			value = json::read_float(n.value, "value", value);
		} else if ("emission_factor" == n.name) {
			emission_factor = json::read_float(n.value);
		} else if ("two_sided" == n.name) {
			two_sided = json::read_bool(n.value);
		} else if ("animation_duration" == n.name) {
			animation_duration = json::read_float(n.value);
		} else if ("textures" == n.name) {
			for (auto& tn : n.value.GetArray()) {
				Texture_description texture_description;
				read_texture_description(tn, texture_description);

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
		} else if ("sampler" == n.name) {
			read_sampler_settings(n.value, sampler_settings);
		}
	}

	if (emission_map.is_valid()) {
		if (animation_duration > 0.f) {
			auto material = std::make_shared<light::Emissionmap_animated>(
						sample_cache_, sampler_settings, two_sided,
						emission_map, emission_factor, animation_duration);
			material->set_mask(mask);
			return material;
		} else {
			auto material = std::make_shared<light::Emissionmap>(sample_cache_, sampler_settings,
																 two_sided);
			material->set_mask(mask);
			material->set_emission_map(emission_map);
			material->set_emission_factor(emission_factor);
			return material;
		}
	}

	auto material = std::make_shared<light::Constant>(sample_cache_, sampler_settings, two_sided);
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

Material_ptr Provider::load_matte(const json::Value& matte_value, resource::Manager& manager) {
	Sampler_settings sampler_settings;

//	Texture_ptr normal_map;
	Texture_adapter mask;
	bool two_sided = false;
	float3 color(0.6f, 0.6f, 0.6f);

	for (auto& n : matte_value.GetObject()) {
		if ("color" == n.name) {
			color = json::read_float3(n.value);
		} else if ("two_sided" == n.name) {
			two_sided = json::read_bool(n.value);
		} else if ("textures" == n.name) {
			for (auto& tn : n.value.GetArray()) {
				Texture_description texture_description;
				read_texture_description(tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				/*if ("Normal" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Normal);
					normal_map = manager.load<image::texture::Texture>(
								texture_description.filename, options);
				} else*/ if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == n.name) {
			read_sampler_settings(n.value, sampler_settings);
		}
	}

	auto material = std::make_shared<matte::Material>(sample_cache_, sampler_settings, two_sided);
	material->set_mask(mask);
//	material->set_normal_map(normal_map);

	material->set_color(color);

	return material;
}

Material_ptr Provider::load_metal(const json::Value& metal_value, resource::Manager& manager) {
	Sampler_settings sampler_settings;

	Texture_adapter normal_map;
//	Texture_ptr surface_map;
	Texture_adapter direction_map;
	Texture_adapter mask;
	bool two_sided = false;
	float3 ior(1.f, 1.f, 1.f);
	float3 absorption(0.75f, 0.75f, 0.75f);
	float roughness = 0.9f;
	float2 roughness_aniso(0.f, 0.f);

	for (auto& n : metal_value.GetObject()) {
		if ("ior" == n.name) {
			ior = json::read_float3(n.value);
		} else if ("absorption" == n.name) {
			absorption = json::read_float3(n.value);
		} else if ("preset" == n.name) {
			metal::ior_and_absorption(n.value.GetString(), ior, absorption);
		} else if ("roughness" == n.name) {
			if (n.value.IsArray()) {
				roughness_aniso = json::read_float2(n.value);
			} else {
				roughness = json::read_float(n.value);
			}
		} else if ("two_sided" == n.name) {
			two_sided = json::read_bool(n.value);
		} else if ("textures" == n.name) {
			for (auto& tn : n.value.GetArray()) {
				Texture_description texture_description;
				read_texture_description(tn, texture_description);

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
		} else if ("sampler" == n.name) {
			read_sampler_settings(n.value, sampler_settings);
		}
	}

	if (roughness_aniso[0] > 0.f && roughness_aniso[1] > 0.f) {
		auto material = std::make_shared<metal::Material_anisotropic>(
					sample_cache_, sampler_settings, two_sided);

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
					sample_cache_, sampler_settings, two_sided);

		material->set_mask(mask);
		material->set_normal_map(normal_map);
	//	material->set_surface_map(surface_map);

		material->set_ior(ior);
		material->set_absorption(absorption);
		material->set_roughness(roughness);

		return material;
	}
}

Material_ptr Provider::load_metallic_paint(const json::Value& paint_value,
										   resource::Manager& manager) {
	Sampler_settings sampler_settings;

	Texture_adapter mask;
	Texture_adapter flakes_normal_map;
	Texture_adapter flakes_mask;
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

	for (auto& n : paint_value.GetObject()) {
		if ("color_a" == n.name) {
			color_a = json::read_float3(n.value);
		} else if ("color_b" == n.name) {
			color_b = json::read_float3(n.value);
		} else if ("roughness" == n.name) {
			roughness  = json::read_float(n.value);
		} else if ("two_sided" == n.name) {
			two_sided = json::read_bool(n.value);
		} else if ("flakes" == n.name) {
			std::string flakes_preset = json::read_string(n.value, "preset");

			if (flakes_preset.empty()) {
				flakes_ior = json::read_float3(n.value, "ior", flakes_ior);
				flakes_absorption = json::read_float3(n.value, "absorption", flakes_absorption);
			} else {
				metal::ior_and_absorption(flakes_preset, flakes_ior, flakes_absorption);
			}

			flakes_size = json::read_float(n.value, "size", flakes_size);
			flakes_density = json::read_float(n.value, "density", flakes_density);
			flakes_roughness = json::read_float(n.value, "roughness", flakes_roughness);
			flakes_scale = json::read_float2(n.value, "scale", flakes_scale);
		} else if ("coating" == n.name) {
			read_coating_description(n.value, coating);
		} else if ("textures" == n.name) {
			for (auto& tn : n.value.GetArray()) {
				Texture_description texture_description;
				read_texture_description(tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == n.name) {
			read_sampler_settings(n.value, sampler_settings);
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

	auto material = std::make_shared<metallic_paint::Material>(sample_cache_, sampler_settings,
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

Material_ptr Provider::load_sky(const json::Value& sky_value, resource::Manager& manager) {
	Sampler_settings sampler_settings;

	Texture_adapter mask;

	bool two_sided = false;

	float3 radiance(0.6f, 0.6f, 0.6f);

	for (auto& n : sky_value.GetObject()) {
		if ("radiance" == n.name) {
			radiance = json::read_float3(n.value);
		} else if ("two_sided" == n.name) {
			two_sided = json::read_bool(n.value);
		} else if ("textures" == n.name) {
			for (auto& tn : n.value.GetArray()) {
				Texture_description texture_description;
				read_texture_description(tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				if ("Mask" == texture_description.usage) {
					options.set("usage", image::texture::Provider::Usage::Mask);
					mask = create_texture(texture_description, options, manager);
				}
			}
		} else if ("sampler" == n.name) {
			read_sampler_settings(n.value, sampler_settings);
		}
	}

	auto material = std::make_shared<sky::Material_overcast>(sample_cache_, sampler_settings,
															 two_sided);

	material->set_mask(mask);
	material->set_emission(radiance);

	return material;
}

Material_ptr Provider::load_substitute(const json::Value& substitute_value,
									   resource::Manager& manager) {
	Sampler_settings sampler_settings;

	Texture_adapter color_map;
	Texture_adapter normal_map;
	Texture_adapter surface_map;
	Texture_adapter emission_map;
	Texture_adapter mask;
	bool two_sided = false;
	float3 color(0.6f, 0.6f, 0.6f);
	float3 absorption(0.f);
	float3 scattering(0.f);
	float roughness = 0.9f;
	float metallic = 0.f;
	float ior = 1.46f;
	float emission_factor = 1.f;
	float thickness = 0.f;
	float attenuation_distance = 0.f;
	Coating_description coating;

	for (auto& n : substitute_value.GetObject()) {
		if ("color" == n.name) {
			color = json::read_float3(n.value);
		} else if ("scattering" == n.name) {
			scattering = json::read_float3(n.value);
		} else if ("absorption" == n.name) {
			absorption = json::read_float3(n.value);
		} else if ("ior" == n.name) {
			ior = json::read_float(n.value);
		} else if ("roughness" == n.name) {
			roughness = json::read_float(n.value);
		} else if ("metallic" == n.name) {
			metallic = json::read_float(n.value);
		} else if ("emission_factor" == n.name) {
			emission_factor = json::read_float(n.value);
		} else if ("thickness" == n.name) {
			thickness = json::read_float(n.value);
		} else if ("attenuation_distance" == n.name) {
			attenuation_distance = json::read_float(n.value);
		} else if ("two_sided" == n.name) {
			two_sided = json::read_bool(n.value);
		} else if ("coating" == n.name) {
			read_coating_description(n.value, coating);
		} else if ("textures" == n.name) {
			for (auto& tn : n.value.GetArray()) {
				Texture_description texture_description;
				read_texture_description(tn, texture_description);

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
		} else if ("sampler" == n.name) {
			read_sampler_settings(n.value, sampler_settings);
		}
	}

	if (thickness > 0.f) {
		auto material = std::make_shared<substitute::Material_translucent>(sample_cache_,
																		   sampler_settings);

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
		Texture_adapter coating_weight_map;
		Texture_adapter coating_normal_map;

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
						sample_cache_, sampler_settings, two_sided);

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
			material->set_thinfilm(coating.ior, coating.roughness, coating.thickness);

			return material;
		} else {
			auto material = std::make_shared<substitute::Material_clearcoat>(
						sample_cache_, sampler_settings, two_sided);

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
	} else if (math::any_greater_zero(scattering)) {
		auto material = std::make_shared<substitute::Material_subsurface>(sample_cache_,
																		  sampler_settings);

		material->set_mask(mask);
		material->set_color_map(color_map);
		material->set_normal_map(normal_map);
		material->set_surface_map(surface_map);
		material->set_emission_map(emission_map);

		material->set_color(color);
		material->set_absorption(absorption);
		material->set_scattering(scattering);
		material->set_ior(ior);
		material->set_roughness(roughness);
		material->set_metallic(metallic);
		material->set_emission_factor(emission_factor);

		return material;
	}

	auto material = std::make_shared<substitute::Material>(sample_cache_, sampler_settings,
														   two_sided);

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

Sampler_settings::Address read_address(const json::Value& address_value) {
	std::string address = json::read_string(address_value);

	if ("Clamp" == address) {
		return Sampler_settings::Address::Clamp;
	} else if ("Repeat" == address) {
		return Sampler_settings::Address::Repeat;
	}

	return Sampler_settings::Address::Unknown;
}

void Provider::read_sampler_settings(const json::Value& sampler_value, Sampler_settings& settings) {
	for (auto& n : sampler_value.GetObject()) {
		if ("filter" == n.name) {
			std::string filter = json::read_string(n.value);

			if ("Nearest" == filter) {
				settings.filter = Sampler_settings::Filter::Nearest;
			} else if ("Linear" == filter) {
				settings.filter = Sampler_settings::Filter::Linear;
			}
		} else if ("address" == n.name) {
			if (n.value.IsArray()) {
				auto address_u = read_address(n.value[0]);
				auto address_v = read_address(n.value[1]);

				settings.address_u = address_u;
				settings.address_v = address_v;
			} else {
				auto address = read_address(n.value);

				settings.address_u = address;
				settings.address_v = address;
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

	for (auto& n : texture_value.GetObject()) {
		if ("file" == n.name) {
			description.filename = json::read_string(n.value);
		} else if ("usage" == n.name) {
			description.usage = json::read_string(n.value);
		} else if ("scale" == n.name) {
			description.scale = json::read_float2(n.value);
		} else if ("num_elements" == n.name) {
			description.num_elements = json::read_int(n.value);
		}
	}
}

Texture_adapter Provider::create_texture(const Texture_description& description,
										 const memory::Variant_map& options,
										 resource::Manager& manager) {
	return Texture_adapter(manager.load<image::texture::Texture>(description.filename, options),
						   description.scale);
}

void Provider::read_coating_description(const json::Value& coating_value,
										Coating_description& description) {
	if (!coating_value.IsObject()) {
		return;
	}

	for (auto& n : coating_value.GetObject()) {
		if ("color" == n.name) {
			description.color = json::read_float3(n.value);
		} else if ("ior" == n.name) {
			description.ior = json::read_float(n.value);
		} else if ("roughness" == n.name) {
			description.roughness = json::read_float(n.value);
		} else if ("thickness" == n.name) {
			description.thickness = json::read_float(n.value);
		} else if ("weight" == n.name) {
			description.weight = json::read_float(n.value);
		} else if ("textures" == n.name) {
			for (auto& tn : n.value.GetArray()) {
				Texture_description texture_description;
				read_texture_description(tn, texture_description);

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

	for (auto& n : spectrum_value.GetObject()) {
		if ("sRGB" == n.name) {
			float3 srgb = json::read_float3(n.value);
			return spectrum::sRGB_to_linear_RGB(srgb);
		} else if ("RGB" == n.name) {
			return json::read_float3(n.value);
		} else if ("temperature" == n.name) {
			float temperature = json::read_float(n.value);
			temperature = std::max(800.f, temperature);
			return spectrum::blackbody(temperature);
		}
	}

	return float3(0.f);
}

uint32_t Provider::max_sample_size() {
	size_t num_bytes = 0;

	num_bytes = std::max(sizeof(cloth::Sample), num_bytes);
	num_bytes = std::max(sizeof(display::Sample), num_bytes);
	num_bytes = std::max(sizeof(glass::Sample), num_bytes);
	num_bytes = std::max(sizeof(glass::Sample_rough), num_bytes);
	num_bytes = std::max(sizeof(glass::Sample_thin), num_bytes);
	num_bytes = std::max(sizeof(light::Sample), num_bytes);
	num_bytes = std::max(sizeof(matte::Sample), num_bytes);
	num_bytes = std::max(sizeof(metal::Sample_anisotropic), num_bytes);
	num_bytes = std::max(sizeof(metal::Sample_isotropic), num_bytes);
	num_bytes = std::max(sizeof(metallic_paint::Sample), num_bytes);
	num_bytes = std::max(sizeof(substitute::Sample), num_bytes);
	num_bytes = std::max(sizeof(substitute::Sample_clearcoat), num_bytes);
	num_bytes = std::max(sizeof(substitute::Sample_subsurface), num_bytes);
	num_bytes = std::max(sizeof(substitute::Sample_thinfilm), num_bytes);
	num_bytes = std::max(sizeof(substitute::Sample_translucent), num_bytes);

	return static_cast<uint32_t>(num_bytes);
}

}}

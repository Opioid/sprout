#include "material_provider.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_manager.inl"
#include "material_sample_cache.inl"
#include "image/texture/texture_2d_provider.hpp"
#include "scene/light/blackbody.hpp"
#include "cloth/cloth_sample.hpp"
#include "cloth/cloth_material.hpp"
#include "display/display_sample.hpp"
#include "display/display_material.hpp"
#include "display/display_material_animated.hpp"
#include "glass/glass_sample.hpp"
#include "glass/glass_material.hpp"
#include "glass/glass_rough_sample.hpp"
#include "glass/glass_rough_material.hpp"
#include "light/light_material_sample.hpp"
#include "light/light_constant.hpp"
#include "light/light_emissionmap.hpp"
#include "light/light_emissionmap_animated.hpp"
#include "metal/metal_sample.hpp"
#include "metal/metal_material.hpp"
#include "sky/sky_material_overcast.hpp"
#include "substitute/substitute_sample.hpp"
#include "substitute/substitute_material.hpp"
#include "substitute/substitute_sample_clearcoat.hpp"
#include "substitute/substitute_material_clearcoat.hpp"
#include "substitute/substitute_sample_translucent.hpp"
#include "substitute/substitute_material_translucent.hpp"
#include "substitute/substitute_material_base.inl"
#include "base/spectrum/rgb.inl"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/memory/variant_map.inl"
#include "base/thread/thread_pool.hpp"

namespace scene { namespace material {

Provider::Provider(uint32_t num_threads) :
	resource::Provider<Material>("Material"),
	cloth_cache_(num_threads),
	display_cache_(num_threads),
	glass_cache_(num_threads),
	glass_rough_cache_(num_threads),
	light_cache_(num_threads),
	metal_iso_cache_(num_threads),
	metal_aniso_cache_(num_threads),
	substitute_cache_(num_threads),
	substitute_clearcoat_cache_(num_threads),
	substitute_translucent_cache_(num_threads) {
	auto material = std::make_shared<substitute::Material>(
				substitute_cache_, nullptr,
				Sampler_settings(Sampler_settings::Filter::Linear), false);
	material->set_color(math::float3(1.f, 0.f, 0.f)),
	material->set_ior(1.45f),
	material->set_roughness(1.f);
	material->set_metallic(0.f);
	fallback_material_ = material;
}

Provider::~Provider() {}

std::shared_ptr<Material> Provider::load(const std::string& filename,
										 const memory::Variant_map& /*options*/,
										 resource::Manager& manager) {
	auto stream_pointer = manager.file_system().read_stream(filename);

	auto root = json::parse(*stream_pointer);

	const json::Value::ConstMemberIterator rendering_node = root->FindMember("rendering");
	if (root->MemberEnd() == rendering_node) {
		throw std::runtime_error("Material has no render node");
	}

	const json::Value& rendering_value = rendering_node->value;

	for (auto n = rendering_value.MemberBegin(); n != rendering_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("Cloth" == node_name) {
			return load_cloth(node_value, manager);
		} else if ("Display" == node_name) {
			return load_display(node_value, manager);
		} else if ("Glass" == node_name) {
			return load_glass(node_value, manager);
		} else if ("Light" == node_name) {
			return load_light(node_value, manager);
		} else if ("Metal" == node_name) {
			return load_metal(node_value, manager);
		} else if ("Sky" == node_name) {
			return load_sky(node_value, manager);
		} else if ("Substitute" == node_name) {
			return load_substitute(node_value, manager);
		}
	}

	throw std::runtime_error("Material is of unknown type");
}

std::shared_ptr<Material> Provider::fallback_material() const {
	return fallback_material_;
}

Generic_sample_cache<light::Sample>& Provider::light_cache() {
	return light_cache_;
}

std::shared_ptr<light::Constant> Provider::create_light() {
	scene::material::Sampler_settings sampler_settings;

	return std::make_shared<light::Constant>(light_cache_, nullptr,
											 sampler_settings, false);
}

std::shared_ptr<Material> Provider::load_cloth(const json::Value& cloth_value,
											   resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	std::shared_ptr<image::texture::Texture_2D> color_map;
	std::shared_ptr<image::texture::Texture_2D> normal_map;
	std::shared_ptr<image::texture::Texture_2D> mask;
	bool two_sided = false;
	math::float3 color(0.75f, 0.75f, 0.75f);

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
					options.insert("usage", image::texture::Provider::Usage::Color);
					color_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				} else if ("Normal" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Normal);
					normal_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				} else if ("Mask" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Mask);
					mask = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	auto material = std::make_shared<cloth::Material>(cloth_cache_, mask,
													  sampler_settings, two_sided);

	material->set_color_map(color_map);
	material->set_normal_map(normal_map);

	material->set_color(color);

	return material;
}

std::shared_ptr<Material> Provider::load_display(const json::Value& display_value,
												 resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	std::shared_ptr<image::texture::Texture_2D> emission_map;
	std::shared_ptr<image::texture::Texture_2D> mask;
	bool two_sided = false;

	math::float3 radiance(10.f, 10.f, 10.f);
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
					options.insert("num_elements", texture_description.num_elements);
				}

				if ("Emission" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Color);
					emission_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);;
				} else if ("Mask" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Mask);
					mask = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);;
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	if (animation_duration > 0.f) {
		auto material = std::make_shared<display::Material_animated>(display_cache_, mask,
																	 sampler_settings, two_sided,
																	 emission_map,
																	 animation_duration);
		material->set_emission_factor(emission_factor);
		material->set_roughness(roughness);
		material->set_ior(ior);
		return material;
	} else {
		auto material = std::make_shared<display::Material>(display_cache_, mask,
															sampler_settings, two_sided);
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

	std::shared_ptr<image::texture::Texture_2D> normal_map;
	math::float3 color(1.f, 1.f, 1.f);
	float attenuation_distance = 1.f;
	float ior = 1.5f;
	float roughness = 0.f;

	for (auto n = glass_value.MemberBegin(); n != glass_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("color" == node_name) {
			color = json::read_float3(node_value);
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
					options.insert("usage", image::texture::Provider::Usage::Normal);
					normal_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	if (roughness >= 0.f) {
		auto material = std::make_shared<glass::Glass>(glass_cache_, nullptr, sampler_settings);
		material->set_normal_map(normal_map);
		material->set_color(color);
		material->set_attenuation_distance(attenuation_distance);
		material->set_ior(ior);
		return material;
	} else {
		auto material = std::make_shared<glass::Glass_rough>(glass_rough_cache_,
															 nullptr, sampler_settings);
		material->set_normal_map(normal_map);
		material->set_color(color);
		material->set_attenuation_distance(attenuation_distance);
		material->set_ior(ior);
		material->set_roughness(roughness);
		return material;
	}
}

std::shared_ptr<Material> Provider::load_light(const json::Value& light_value,
											   resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	std::string quantity;
	math::float3 color(1.f, 1.f, 1.f);
	float value = 1.f;

	math::float3 radiance(10.f, 10.f, 10.f);
	float emission_factor = 1.f;
	float animation_duration = 0.f;

	std::shared_ptr<image::texture::Texture_2D> emission_map;
	std::shared_ptr<image::texture::Texture_2D> mask;
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

			value = json::read_float(node_value, "value");
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
					options.insert("num_elements", texture_description.num_elements);
				}

				if ("Emission" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Color);
					emission_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				} else if ("Mask" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Mask);
					mask = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	if (emission_map) {
		if (animation_duration > 0.f) {
			return std::make_shared<light::Emissionmap_animated>(light_cache_, mask,
																 sampler_settings, two_sided,
																 emission_map, emission_factor,
																 animation_duration);
		} else {
			auto material = std::make_shared<light::Emissionmap>(light_cache_, mask,
																 sampler_settings, two_sided);
			material->set_emission_map(emission_map);
			material->set_emission_factor(emission_factor);
			return material;
		}
	}

	auto material = std::make_shared<light::Constant>(light_cache_, mask,
													  sampler_settings, two_sided);
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

std::shared_ptr<Material> Provider::load_metal(const json::Value& substitute_value,
											   resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

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
		const json::Value& node_value = n->value;

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
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				if ("Normal" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Normal);
					normal_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
			/*	} else if ("Surface" == usage) {
					surface_map = texture_cache_.load(filename,
													  static_cast<uint32_t>(
														 image::texture::Provider::Flags::Use_as_surface));*/
				} else if ("Anisotropy" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Anisotropy);
					direction_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				} else if ("Mask" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Mask);
					mask = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	if (roughness_aniso.x > 0.f && roughness_aniso.y > 0.f) {
		auto material = std::make_shared<metal::Material_anisotropic>(metal_aniso_cache_, mask,
																	  sampler_settings, two_sided);

		material->set_normal_map(normal_map);
	//	material->set_surface_map(surface_map);
		material->set_direction_map(direction_map);

		material->set_ior(ior);
		material->set_absorption(absorption);
		material->set_roughness(roughness_aniso);

		return material;
	} else {
		auto material = std::make_shared<metal::Material_isotropic>(metal_iso_cache_, mask,
																	sampler_settings, two_sided);

		material->set_normal_map(normal_map);
	//	material->set_surface_map(surface_map);

		material->set_ior(ior);
		material->set_absorption(absorption);
		material->set_roughness(roughness);

		return material;
	}
}

std::shared_ptr<Material> Provider::load_sky(const json::Value& sky_value,
											 resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	std::shared_ptr<image::texture::Texture_2D> mask;

	bool two_sided = false;

	math::float3 radiance(0.6f, 0.6f, 0.6f);

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
					options.insert("usage", image::texture::Provider::Usage::Mask);
					mask = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	auto material = std::make_shared<sky::Material_overcast>(light_cache_, mask,
															 sampler_settings, two_sided);

	material->set_emission(radiance);

	return material;
}

std::shared_ptr<Material> Provider::load_substitute(const json::Value& substitute_value,
													resource::Manager& manager) {
	scene::material::Sampler_settings sampler_settings;

	std::shared_ptr<image::texture::Texture_2D> color_map;
	std::shared_ptr<image::texture::Texture_2D> normal_map;
	std::shared_ptr<image::texture::Texture_2D> surface_map;
	std::shared_ptr<image::texture::Texture_2D> emission_map;
	std::shared_ptr<image::texture::Texture_2D> mask;
	bool two_sided = false;
    math::float3 color(0.6f, 0.6f, 0.6f);
	float roughness = 0.9f;
	float metallic = 0.f;
	float ior = 1.46f;
	float emission_factor = 1.f;
	float thickness = 0.f;
	float attenuation_distance = 0.f;
	Clearcoat_description clearcoat;

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
		} else if ("clearcoat" == node_name) {
			read_clearcoat_description(node_value, clearcoat);
		} else if ("textures" == node_name) {
			for (auto tn = node_value.Begin(); tn != node_value.End(); ++tn) {
				Texture_description texture_description;
				read_texture_description(*tn, texture_description);

				if (texture_description.filename.empty()) {
					continue;
				}

				memory::Variant_map options;
				if ("Color" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Color);
					color_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				} else if ("Normal" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Normal);
					normal_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				} else if ("Surface" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Surface);
					surface_map =manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				} else if ("Emission" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Color);
					emission_map = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				} else if ("Mask" == texture_description.usage) {
					options.insert("usage", image::texture::Provider::Usage::Mask);
					mask = manager.load<image::texture::Texture_2D>(
								texture_description.filename, options);
				}
			}
		} else if ("sampler" == node_name) {
			read_sampler_settings(node_value, sampler_settings);
		}
	}

	if (thickness > 0.f) {
		auto material = std::make_shared<substitute::Material_translucent>(
					substitute_translucent_cache_, mask, sampler_settings, two_sided);

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
	} else if (clearcoat.ior > 1.f) {
		auto material = std::make_shared<substitute::Material_clearcoat>(
					substitute_clearcoat_cache_, mask, sampler_settings, two_sided);

		material->set_color_map(color_map);
		material->set_normal_map(normal_map);
		material->set_surface_map(surface_map);
		material->set_emission_map(emission_map);

		material->set_color(color);
		material->set_ior(ior);
		material->set_roughness(roughness);
		material->set_metallic(metallic);
		material->set_emission_factor(emission_factor);
		material->set_clearcoat(clearcoat.ior, clearcoat.roughness);

		return material;
	}

	auto material = std::make_shared<substitute::Material>(
				substitute_cache_, mask, sampler_settings, two_sided);

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
	description.num_elements = 1;

	for (auto n = texture_value.MemberBegin(); n != texture_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("file" == node_name) {
			description.filename = json::read_string(node_value);
		} else if ("usage" == node_name) {
			description.usage = json::read_string(node_value);
		} else if ("num_elements" == node_name) {
			description.num_elements = json::read_int(node_value);
		}
	}
}

void Provider::read_clearcoat_description(const json::Value& clearcoat_value,
										  Clearcoat_description& description) {
	if (!clearcoat_value.IsObject()) {
		return;
	}

	for (auto n = clearcoat_value.MemberBegin(); n != clearcoat_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("ior" == node_name) {
			description.ior = json::read_float(node_value);
		} else if ("roughness" == node_name) {
			description.roughness = json::read_float(node_value);
		}
	}
}

math::float3 Provider::read_spectrum(const json::Value& spectrum_value) {
	if (!spectrum_value.IsObject()) {
		return math::float3(0.f, 0.f, 0.f);
	}

	for (auto n = spectrum_value.MemberBegin(); n != spectrum_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("sRGB" == node_name) {
			math::float3 srgb = json::read_float3(node_value);
			return spectrum::sRGB_to_linear(srgb);
		} else if ("temperature" == node_name) {
			float temperature = json::read_float(node_value);
			temperature = std::max(1667.f, temperature);
			return scene::light::blackbody(temperature);
		}
	}

	return math::float3(0.f, 0.f, 0.f);
}

}}

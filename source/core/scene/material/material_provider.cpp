#include "material_provider.hpp"
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "base/memory/array.inl"
#include "base/memory/variant_map.inl"
#include "base/spectrum/blackbody.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "cloth/cloth_material.hpp"
#include "display/display_constant.hpp"
#include "display/display_emissionmap.hpp"
#include "display/display_emissionmap_animated.hpp"
#include "glass/glass_dispersion_material.hpp"
#include "glass/glass_material.hpp"
#include "glass/glass_rough_material.hpp"
#include "glass/glass_thin_material.hpp"
#include "image/texture/texture.inl"
#include "image/texture/texture_adapter.inl"
#include "image/texture/texture_provider.hpp"
#include "light/light_constant.hpp"
#include "light/light_emissionmap.hpp"
#include "light/light_emissionmap_animated.hpp"
#include "logging/logging.hpp"
#include "material_sample_cache.inl"
#include "matte/matte_material.hpp"
#include "metal/metal_material.hpp"
#include "metal/metal_presets.hpp"
#include "metallic_paint/metallic_paint_material.hpp"
#include "mix/mix_material.hpp"
#include "resource/resource_manager.inl"
#include "resource/resource_provider.inl"
#include "scene/scene_constants.hpp"
#include "sky/sky_material_overcast.hpp"
#include "substitute/substitute_coating_material.inl"
#include "substitute/substitute_coating_subsurface_material.hpp"
#include "substitute/substitute_material.hpp"
#include "substitute/substitute_subsurface_material.hpp"
#include "substitute/substitute_translucent_material.hpp"
#include "volumetric/volumetric_grid.hpp"
#include "volumetric/volumetric_height.hpp"
#include "volumetric/volumetric_homogeneous.hpp"

// #define FROZEN 1

namespace scene::material {

using Texture       = image::texture::Texture;
using Texture_usage = image::texture::Provider::Usage;
using Resources     = resource::Manager;

static Material* load_substitute(json::Value const& substitute_value,
                                 Resources&         resources) noexcept;

struct Texture_description {
    std::string filename;

    std::string usage = "Color";

    image::Swizzle swizzle = image::Swizzle::XYZW;

    float scale = 1.f;

    int32_t num_elements = 1;
};

static void read_sampler_settings(json::Value const& sampler_value,
                                  Sampler_settings&  settings) noexcept;

static Texture_description read_texture_description(json::Value const& texture_value) noexcept;

static Texture_adapter create_texture(Texture_description const& description,
                                      memory::Variant_map&       options,
                                      resource::Manager&         resources) noexcept;

struct Coating_description {
    float3 color = float3(1.f);

    float attenuation_distance = 1.f;
    float ior                  = 1.f;
    float roughness            = 0.f;
    float thickness            = 0.001f;

    bool in_nm = false;

    Texture_description normal_map_description;
    Texture_description thickness_map_description;
};

static void read_coating_description(json::Value const&   value,
                                     Coating_description& description) noexcept;

static float3 read_hex_RGB(std::string const& text) noexcept;

static float3 read_color(json::Value const& color_value) noexcept;

static float3 read_spectrum(json::Value const& spectrum_value) noexcept;

Provider::Provider(bool force_debug_material) noexcept
    : force_debug_material_(force_debug_material) {
    Material::init_rainbow();
}

Provider::~Provider() noexcept = default;

Material* Provider::load(std::string const& filename, Variants const& /*options*/,
                         Resources& resources, std::string& resolved_name) noexcept {
    auto stream_pointer = resources.filesystem().read_stream(filename, resolved_name);
    if (!stream_pointer) {
        return nullptr;
    }

    std::string error;
    auto        root = json::parse(*stream_pointer, error);
    if (!root) {
        logging::push_error(error);
        return nullptr;
    }

    return load(*root, string::parent_directory(resolved_name), resources);
}

Material* Provider::load(void const* data, std::string const&    source_name,
                         Variants const& /*options*/, Resources& resources) noexcept {
    json::Value const* value = reinterpret_cast<json::Value const*>(data);

    return load(*value, string::parent_directory(source_name), resources);
}

size_t Provider::num_bytes() const noexcept {
    return sizeof(*this);
}

size_t Provider::num_bytes(Material const* resource) const noexcept {
    return resource->num_bytes();
}

Material* Provider::create_fallback_material() noexcept {
    return new debug::Material(Sampler_settings(Sampler_settings::Filter::Linear));
}

Material* Provider::load(json::Value const& value, std::string_view mount_folder,
                         Resources& resources) noexcept {
    json::Value::ConstMemberIterator const rendering_node = value.FindMember("rendering");
    if (value.MemberEnd() == rendering_node) {
        logging::push_error("Material has no render node.");
        return nullptr;
    }

    resources.filesystem().push_mount(mount_folder);

    Material* material = nullptr;

    json::Value const& rendering_value = rendering_node->value;

    for (auto const& n : rendering_value.GetObject()) {
        if (force_debug_material_) {
            if ("Light" == n.name) {
                material = load_light(n.value, resources);
            } else {
                material = new debug::Material(Sampler_settings(Sampler_settings::Filter::Linear));
            }
        } else {
            if ("Cloth" == n.name) {
                material = load_cloth(n.value, resources);
            } else if ("Debug" == n.name) {
                material = load_debug(n.value, resources);
            } else if ("Display" == n.name) {
                material = load_display(n.value, resources);
            } else if ("Glass" == n.name) {
                material = load_glass(n.value, resources);
            } else if ("Light" == n.name) {
                material = load_light(n.value, resources);
            } else if ("Matte" == n.name) {
                material = load_matte(n.value, resources);
            } else if ("Metal" == n.name) {
                material = load_metal(n.value, resources);
            } else if ("Metallic_paint" == n.name) {
                material = load_metallic_paint(n.value, resources);
            } else if ("Mix" == n.name) {
                material = load_mix(n.value, resources);
            } else if ("Sky" == n.name) {
                material = load_sky(n.value, resources);
            } else if ("Substitute" == n.name) {
                material = load_substitute(n.value, resources);
            } else if ("Volumetric" == n.name) {
                material = load_volumetric(n.value, resources);
            }
        }

        if (material) {
            break;
        }
    }

    resources.filesystem().pop_mount();

    if (!material) {
        logging::push_error("Material is of unknown type.");
        return nullptr;
    }

    return material;
}

Material* Provider::load_cloth(json::Value const& cloth_value, Resources& resources) noexcept {
    Sampler_settings sampler_settings;

    Texture_adapter color_map;
    Texture_adapter normal_map;
    Texture_adapter mask;

    bool   two_sided = false;
    float3 color(0.75f, 0.75f, 0.75f);

    for (auto const& n : cloth_value.GetObject()) {
        if ("color" == n.name) {
            color = read_color(n.value);
        } else if ("two_sided" == n.name) {
            two_sided = json::read_bool(n.value);
        } else if ("textures" == n.name) {
            for (auto const& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;
                if ("Color" == texture_description.usage) {
                    options.set("usage", Texture_usage::Color);
                    color_map = create_texture(texture_description, options, resources);
                } else if ("Normal" == texture_description.usage) {
                    options.set("usage", Texture_usage::Normal);
                    normal_map = create_texture(texture_description, options, resources);
                } else if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    auto material = new cloth::Material(sampler_settings, two_sided);

    material->set_mask(mask);
    material->set_color_map(color_map);
    material->set_normal_map(normal_map);

    material->set_color(color);

    return material;
}

Material* Provider::load_debug(json::Value const& debug_value, Resources& resources) noexcept {
    Sampler_settings sampler_settings;

    Texture_adapter mask;

    for (auto const& n : debug_value.GetObject()) {
        if ("textures" == n.name) {
            for (auto const& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;
                if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    auto material = new debug::Material(sampler_settings);

    material->set_mask(mask);

    return material;
}

Material* Provider::load_display(json::Value const& display_value, Resources& resources) noexcept {
    Sampler_settings sampler_settings;

    Texture_adapter mask;
    Texture_adapter emission_map;
    bool            two_sided = false;

    float3 radiance(10.f);

    float emission_factor = 1.f;
    float roughness       = 1.f;
    float ior             = 1.5;

    uint64_t animation_duration = 0;

    for (auto const& n : display_value.GetObject()) {
        if ("radiance" == n.name) {
            radiance = read_color(n.value);
        } else if ("emission_factor" == n.name) {
            emission_factor = json::read_float(n.value);
        } else if ("roughness" == n.name) {
            roughness = json::read_float(n.value);
        } else if ("ior" == n.name) {
            ior = json::read_float(n.value);
        } else if ("two_sided" == n.name) {
            two_sided = json::read_bool(n.value);
        } else if ("animation_duration" == n.name) {
            animation_duration = scene::time(json::read_double(n.value));
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;

                if ("Emission" == texture_description.usage) {
                    options.set("usage", Texture_usage::Color);
                    emission_map = create_texture(texture_description, options, resources);
                } else if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    if (emission_map.is_valid()) {
        if (animation_duration > 0) {
            auto material = new display::Emissionmap_animated(sampler_settings, two_sided);
            material->set_mask(mask);
            material->set_emission_map(emission_map, animation_duration);
            material->set_emission_factor(emission_factor);
            material->set_roughness(roughness);
            material->set_ior(ior);
            return material;
        }

        auto material = new display::Emissionmap(sampler_settings, two_sided);
        material->set_mask(mask);
        material->set_emission_map(emission_map);
        material->set_emission_factor(emission_factor);
        material->set_roughness(roughness);
        material->set_ior(ior);
        return material;
    }

    auto material = new display::Constant(sampler_settings, two_sided);
    material->set_mask(mask);
    material->set_emission(radiance);
    material->set_roughness(roughness);
    material->set_ior(ior);
    return material;
}

Material* Provider::load_glass(json::Value const& glass_value, Resources& resources) noexcept {
    Sampler_settings sampler_settings;

    Texture_adapter mask;
    Texture_adapter normal_map;
    Texture_adapter roughness_map;

    float3 refraction_color(1.f);
    float3 absorption_color(1.f);

    float attenuation_distance = 1.f;
    float ior                  = 1.5f;
    float abbe                 = 0.f;
    float roughness            = 0.f;
    float thickness            = 0.f;

    for (auto const& n : glass_value.GetObject()) {
        if ("color" == n.name || "absorption_color" == n.name) {
            absorption_color = read_color(n.value);
        } else if ("refraction_color" == n.name) {
            refraction_color = read_color(n.value);
        } else if ("attenuation_distance" == n.name) {
            attenuation_distance = json::read_float(n.value);
        } else if ("ior" == n.name) {
            ior = json::read_float(n.value);
        } else if ("abbe" == n.name) {
            abbe = json::read_float(n.value);
        } else if ("roughness" == n.name) {
            roughness = json::read_float(n.value);
        } else if ("thickness" == n.name) {
            thickness = json::read_float(n.value);
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;

                if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
                } else if ("Normal" == texture_description.usage) {
                    options.set("usage", Texture_usage::Normal);
                    normal_map = create_texture(texture_description, options, resources);
                } else if ("Roughness" == texture_description.usage) {
                    options.set("usage", Texture_usage::Roughness);
                    roughness_map = create_texture(texture_description, options, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    if (roughness > 0.f || roughness_map.is_valid()) {
        auto material = new glass::Glass_rough(sampler_settings);
        material->set_normal_map(normal_map);
        material->set_roughness_map(roughness_map);
        material->set_refraction_color(refraction_color);
        material->set_attenuation(absorption_color, attenuation_distance);
        material->set_ior(ior);
        material->set_roughness(roughness);
        return material;
    }

    if (thickness > 0.f) {
        auto material = new glass::Glass_thin(sampler_settings);
        material->set_mask(mask);
        material->set_normal_map(normal_map);
        material->set_refraction_color(refraction_color);
        material->set_attenuation(absorption_color, attenuation_distance);
        material->set_ior(ior);
        material->set_thickness(thickness);
        return material;
    }

    if (abbe > 0.f) {
        auto material = new glass::Glass_dispersion(sampler_settings);
        material->set_normal_map(normal_map);
        material->set_refraction_color(refraction_color);
        material->set_attenuation(absorption_color, attenuation_distance);
        material->set_ior(ior);
        material->set_abbe(abbe);
        return material;
    }

    auto material = new glass::Glass(sampler_settings);
    material->set_normal_map(normal_map);
    material->set_refraction_color(refraction_color);
    material->set_attenuation(absorption_color, attenuation_distance);
    material->set_ior(ior);
    return material;
}

Material* Provider::load_light(json::Value const& light_value, Resources& resources) noexcept {
    Sampler_settings sampler_settings;

    std::string quantity;
    float3      color(1.f);
    float       value = 1.f;

    float3 radiance(10.f);
    float  emission_factor = 1.f;

    uint64_t animation_duration = 0;

    Texture_adapter emission_map;
    Texture_adapter mask;
    bool            two_sided = false;

    for (auto const& n : light_value.GetObject()) {
        if ("emission" == n.name) {
            radiance = read_color(n.value);
        } else if ("emittance" == n.name) {
            quantity = json::read_string(n.value, "quantity");

            auto const s = n.value.FindMember("spectrum");
            if (n.value.MemberEnd() != s) {
                color = read_spectrum(s->value);
            }

            value = json::read_float(n.value, "value", value);
        } else if ("emission_factor" == n.name) {
            emission_factor = json::read_float(n.value);
        } else if ("two_sided" == n.name) {
            two_sided = json::read_bool(n.value);
        } else if ("animation_duration" == n.name) {
            animation_duration = time(json::read_double(n.value));
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;

                if ("Emission" == texture_description.usage) {
                    options.set("usage", Texture_usage::Color);
                    emission_map = create_texture(texture_description, options, resources);
                } else if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    if (emission_map.is_valid()) {
        if (animation_duration > 0) {
            auto material = new light::Emissionmap_animated(sampler_settings, two_sided);
            material->set_mask(mask);
            material->set_emission_map(emission_map, animation_duration);
            material->set_emission_factor(emission_factor);
            return material;
        }
        auto material = new light::Emissionmap(sampler_settings, two_sided);
        material->set_mask(mask);
        material->set_emission_map(emission_map);
        material->set_emission_factor(emission_factor);
        return material;
    }

    auto material = new light::Constant(sampler_settings, two_sided);
    material->set_mask(mask);

    if ("Flux" == quantity) {
        material->emittance().set_luminous_flux(color, value);
    } else if ("Intensity" == quantity) {
        material->emittance().set_luminous_intensity(color, value);
    } else if ("Exitance" == quantity) {
        material->emittance().set_luminous_exitance(color, value);
    } else if ("Luminance" == quantity) {
        material->emittance().set_luminance(color, value);
    } else if ("Radiance" == quantity) {
        material->emittance().set_radiance(value * color);
    } else {
        material->emittance().set_radiance(radiance);
    }

    return material;
}

Material* Provider::load_matte(json::Value const& matte_value, Resources& resources) noexcept {
    Sampler_settings sampler_settings;

    //	Texture_ptr normal_map;
    Texture_adapter mask;

    bool   two_sided = false;
    float3 color(0.6f, 0.6f, 0.6f);

    for (auto const& n : matte_value.GetObject()) {
        if ("color" == n.name) {
            color = read_color(n.value);
        } else if ("two_sided" == n.name) {
            two_sided = json::read_bool(n.value);
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;
                /*if ("Normal" == texture_description.usage) {
                        options.insert("usage", Texture_usage::Normal);
                        normal_map = resources.load<image::texture::Texture(
                                                texture_description.filename, options);
                } else*/
                if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    auto material = new matte::Material(sampler_settings, two_sided);
    material->set_mask(mask);
    //	material->set_normal_map(normal_map);

    material->set_color(color);

    return material;
}

Material* Provider::load_metal(json::Value const& metal_value, Resources& resources) noexcept {
    Sampler_settings sampler_settings;

    Texture_adapter normal_map;
    //	Texture_ptr surface_map;
    Texture_adapter direction_map;
    Texture_adapter mask;

    bool   two_sided = false;
    float3 ior(1.f, 1.f, 1.f);
    float3 absorption(0.75f, 0.75f, 0.75f);
    float  roughness = 0.9f;
    float2 roughness_aniso(0.f, 0.f);

    for (auto const& n : metal_value.GetObject()) {
        if ("ior" == n.name) {
            ior = read_color(n.value);
        } else if ("absorption" == n.name) {
            absorption = read_color(n.value);
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
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;
                if ("Normal" == texture_description.usage) {
                    options.set("usage", Texture_usage::Normal);
                    normal_map = create_texture(texture_description, options, resources);
                } else if ("Anisotropy" == texture_description.usage) {
                    options.set("usage", Texture_usage::Anisotropy);
                    direction_map = create_texture(texture_description, options, resources);
                } else if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    if (roughness_aniso[0] > 0.f && roughness_aniso[1] > 0.f) {
        auto material = new metal::Material_anisotropic(sampler_settings, two_sided);

        material->set_mask(mask);
        material->set_normal_map(normal_map);
        //	material->set_surface_map(surface_map);
        material->set_direction_map(direction_map);

        material->set_ior(ior);
        material->set_absorption(absorption);
        material->set_roughness(roughness_aniso);

        return material;
    }

    auto material = new metal::Material_isotropic(sampler_settings, two_sided);

    material->set_mask(mask);
    material->set_normal_map(normal_map);
    //	material->set_surface_map(surface_map);

    material->set_ior(ior);
    material->set_absorption(absorption);
    material->set_roughness(roughness);

    return material;
}

Material* Provider::load_metallic_paint(json::Value const& paint_value,
                                        Resources&         resources) noexcept {
    Sampler_settings sampler_settings;

    Texture_adapter mask;
    Texture_adapter flakes_normal_map;
    Texture_adapter flakes_mask;

    bool   two_sided = false;
    float3 color_a(1.f, 0.f, 0.f);
    float3 color_b(0.f, 0.f, 1.f);
    float  roughness      = 0.575f;
    float  flakes_size    = 0.1f;
    float  flakes_density = 0.2f;
    float3 flakes_ior(1.f);
    float3 flakes_absorption(0.75f);
    float  flakes_roughness = 0.3f;
    float  flakes_scale     = 1.f;

    Coating_description coating;
    coating.ior = 1.5f;

    for (auto const& n : paint_value.GetObject()) {
        if ("color_a" == n.name) {
            color_a = read_color(n.value);
        } else if ("color_b" == n.name) {
            color_b = read_color(n.value);
        } else if ("roughness" == n.name) {
            roughness = json::read_float(n.value);
        } else if ("two_sided" == n.name) {
            two_sided = json::read_bool(n.value);
        } else if ("flakes" == n.name) {
            std::string flakes_preset = json::read_string(n.value, "preset");

            if (flakes_preset.empty()) {
                flakes_ior        = json::read_float3(n.value, "ior", flakes_ior);
                flakes_absorption = json::read_float3(n.value, "absorption", flakes_absorption);
            } else {
                metal::ior_and_absorption(flakes_preset, flakes_ior, flakes_absorption);
            }

            flakes_size      = json::read_float(n.value, "size", flakes_size);
            flakes_density   = json::read_float(n.value, "density", flakes_density);
            flakes_roughness = json::read_float(n.value, "roughness", flakes_roughness);
            flakes_scale     = json::read_float(n.value, "scale", flakes_scale);
        } else if ("coating" == n.name) {
            read_coating_description(n.value, coating);
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;
                if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
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
    options.set("usage", Texture_usage::Normal);
    flakes_normal_map = create_texture(texture_description, options, resources);

    texture_description.filename = "proc:flakes_mask";
    options.set("usage", Texture_usage::Mask);
    flakes_mask = create_texture(texture_description, options, resources);

    auto material = new metallic_paint::Material(sampler_settings, two_sided);

    material->set_mask(mask);

    material->set_color(color_a, color_b);
    material->set_roughness(roughness);

    material->set_flakes_mask(flakes_mask);
    material->set_flakes_normal_map(flakes_normal_map);
    material->set_flakes_ior(flakes_ior);
    material->set_flakes_absorption(flakes_absorption);
    material->set_flakes_roughness(flakes_roughness);

    material->set_coating_attenuation(coating.color, coating.attenuation_distance);
    material->set_coating_ior(coating.ior);
    material->set_coating_roughness(coating.roughness);
    material->set_coating_thickness(coating.thickness);

    return material;
}

Material* Provider::load_mix(json::Value const& mix_value, Resources& resources) noexcept {
    Sampler_settings sampler_settings;

    Texture_adapter mask;

    bool two_sided = false;

    memory::Array<material::Material*> materials;
    materials.reserve(2);

    for (auto const& n : mix_value.GetObject()) {
        if ("materials" == n.name) {
            for (auto& m : n.value.GetArray()) {
                if (materials.full()) {
                    continue;
                }

                std::string const filename = json::read_string(m, "file");

                if (!filename.empty()) {
                    materials.push_back(resources.load<Material>(filename).ptr);
                } else {
                    materials.push_back(load(m, "", resources));
                }
            }
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;
                if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    if (materials.size() < 2) {
        logging::push_error("Mix material needs 2 child materials.");
        return nullptr;
    }

    //    if (!mask.is_valid()) {
    //        return materials[0];
    //    }

    auto material = new mix::Material(sampler_settings, two_sided);

    material->set_mask(mask);

    material->set_materials(materials[0], materials[1]);

    return material;
}

Material* Provider::load_sky(json::Value const& sky_value, Resources& resources) noexcept {
    Sampler_settings sampler_settings;

    Texture_adapter mask;

    bool two_sided = false;

    float3 radiance(0.6f, 0.6f, 0.6f);

    for (auto const& n : sky_value.GetObject()) {
        if ("radiance" == n.name) {
            radiance = read_color(n.value);
        } else if ("two_sided" == n.name) {
            two_sided = json::read_bool(n.value);
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;
                if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    auto material = new sky::Material_overcast(sampler_settings, two_sided);

    material->set_mask(mask);
    material->set_emission(radiance);

    return material;
}

Material* load_substitute(json::Value const& substitute_value, Resources& resources) noexcept {
    Sampler_settings sampler_settings;

    Texture_adapter color_map;
    Texture_adapter normal_map;
    Texture_adapter surface_map;
    Texture_adapter emission_map;
    Texture_adapter mask;
    Texture_adapter density_map;

    bool   two_sided = false;
    float3 color(0.6f, 0.6f, 0.6f);
    bool   use_absorption_color = false;
    float3 absorption_color(0.f);
    bool   use_scattering_color = false;
    float3 scattering_color(0.f);

    float roughness             = 0.9f;
    float metallic              = 0.f;
    float ior                   = 1.46f;
    float emission_factor       = 1.f;
    float thickness             = 0.f;
    float attenuation_distance  = 0.f;
    float volumetric_anisotropy = 0.f;

    Coating_description coating;

    for (auto const& n : substitute_value.GetObject()) {
        if ("color" == n.name) {
            color = read_color(n.value);
        } else if ("metal_preset" == n.name) {
            float3 eta;
            float3 k;
            metal::ior_and_absorption(n.value.GetString(), eta, k);

            color = fresnel::conductor(1.f, eta, k);

            metallic = 1.f;

            //            float3 const f82 = fresnel::conductor(1.f / 7.f, eta, k);
            //            a                = fresnel::lazanyi_schlick_a(f0, f82);
        } else if ("absorption_color" == n.name) {
            use_absorption_color = true;
            absorption_color     = read_color(n.value);
        } else if ("scattering_color" == n.name) {
            use_scattering_color = true;
            scattering_color     = read_color(n.value);
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
        } else if ("volumetric_anisotropy" == n.name) {
            volumetric_anisotropy = json::read_float(n.value);
        } else if ("two_sided" == n.name) {
            two_sided = json::read_bool(n.value);
        } else if ("coating" == n.name) {
            read_coating_description(n.value, coating);
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;
                if ("Color" == texture_description.usage) {
                    options.set("usage", Texture_usage::Color);
                    color_map = create_texture(texture_description, options, resources);
                } else if ("Normal" == texture_description.usage) {
                    options.set("usage", Texture_usage::Normal);
                    normal_map = create_texture(texture_description, options, resources);
                } else if ("Surface" == texture_description.usage) {
                    options.set("usage", Texture_usage::Surface);
                    surface_map = create_texture(texture_description, options, resources);
                } else if ("Roughness" == texture_description.usage) {
                    options.set("usage", Texture_usage::Roughness);
                    surface_map = create_texture(texture_description, options, resources);
                } else if ("Gloss" == texture_description.usage) {
                    options.set("usage", Texture_usage::Gloss);
                    surface_map = create_texture(texture_description, options, resources);
                } else if ("Gloss_in_alpha" == texture_description.usage) {
                    options.set("usage", Texture_usage::Gloss_in_alpha);
                    surface_map = create_texture(texture_description, options, resources);
                } else if ("Emission" == texture_description.usage) {
                    options.set("usage", Texture_usage::Color);
                    emission_map = create_texture(texture_description, options, resources);
                } else if ("Mask" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    mask = create_texture(texture_description, options, resources);
                } else if ("Density" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    density_map = create_texture(texture_description, options, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    if (thickness > 0.f) {
        auto material = new substitute::Material_translucent(sampler_settings);

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
    }

    if (coating.ior > 1.f) {
        Texture_adapter coating_thickness_map;
        Texture_adapter coating_normal_map;

        if (!coating.thickness_map_description.filename.empty()) {
            memory::Variant_map options;
            options.set("usage", Texture_usage::Mask);
            coating_thickness_map = create_texture(coating.thickness_map_description, options,
                                                   resources);
        }

        if (!coating.normal_map_description.filename.empty()) {
            memory::Variant_map options;
            options.set("usage", Texture_usage::Normal);
            coating_normal_map = create_texture(coating.normal_map_description, options, resources);
        }

        if (coating.in_nm) {
            auto material = new substitute::Material_thinfilm(sampler_settings, two_sided);

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
            material->set_coating_thickness_map(coating_thickness_map);
            material->set_thinfilm(coating.ior, coating.roughness, coating.thickness);

            return material;
        }

        if (attenuation_distance > 0.f || density_map.is_valid()) {
            auto material = new substitute::Material_coating_subsurface(sampler_settings);

            material->set_mask(mask);
            material->set_color_map(color_map);
            material->set_normal_map(normal_map);
            material->set_surface_map(surface_map);
            material->set_emission_map(emission_map);
            material->set_density_map(density_map);

            material->set_color(color);
            material->set_attenuation(use_absorption_color ? absorption_color : color,
                                      use_scattering_color ? scattering_color : color,
                                      attenuation_distance);
            material->set_volumetric_anisotropy(volumetric_anisotropy);
            material->set_ior(ior);
            material->set_roughness(roughness);
            material->set_metallic(metallic);
            material->set_emission_factor(emission_factor);

            material->set_coating_normal_map(coating_normal_map);
            material->set_coating_thickness_map(coating_thickness_map);
            material->set_coating_attenuation(coating.color, coating.attenuation_distance);
            material->set_coating_ior(coating.ior);
            material->set_coating_roughness(coating.roughness);
            material->set_coating_thickness(coating.thickness);

            return material;
        }

        auto material = new substitute::Material_clearcoat(sampler_settings, two_sided);

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
        material->set_coating_thickness_map(coating_thickness_map);
        material->set_coating_attenuation(coating.color, coating.attenuation_distance);
        material->set_coating_ior(coating.ior);
        material->set_coating_roughness(coating.roughness);
        material->set_coating_thickness(coating.thickness);

        return material;
    }

    if (attenuation_distance > 0.f || density_map.is_valid()) {
        auto material = new substitute::Material_subsurface(sampler_settings);

        material->set_mask(mask);
        material->set_color_map(color_map);
        material->set_normal_map(normal_map);
        material->set_surface_map(surface_map);
        material->set_emission_map(emission_map);
        material->set_density_map(density_map);

        material->set_color(color);
        material->set_attenuation(use_absorption_color ? absorption_color : color,
                                  use_scattering_color ? scattering_color : color,
                                  attenuation_distance);
        material->set_volumetric_anisotropy(volumetric_anisotropy);
        material->set_ior(ior);
        material->set_roughness(roughness);
        material->set_metallic(metallic);
        material->set_emission_factor(emission_factor);

        return material;
    }

#ifdef FROZEN
    if (!emission_map.is_valid()) {
        Texture_description texture_description;
        texture_description.scale = 1.f;

        //        memory::Variant_map options;
        //        options.set("usage", Texture_usage::Normal);

        //        texture_description.filename = "textures/snow_noise.png";
        //        options.set("usage", Texture_usage::Normal);
        //        Texture_adapter snow_normal_map = create_texture(texture_description, options,
        //        resources);

        memory::Variant_map options;
        options.set("size", 0.01f);
        options.set("density", 0.001f);

        texture_description.filename = "proc:flakes";
        options.set("usage", Texture_usage::Normal);
        Texture_adapter snow_normal_map = create_texture(texture_description, options, resources);

        texture_description.filename = "proc:flakes_mask";
        options.set("usage", Texture_usage::Mask);
        Texture_adapter snow_mask = create_texture(texture_description, options, resources);

        auto material = new substitute::Frozen(sampler_settings, two_sided);

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

        material->set_snow_normal_map(snow_normal_map);
        material->set_snow_mask(snow_mask);

        return material;
    }
#endif

    auto material = new substitute::Material(sampler_settings, two_sided);

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

Material* Provider::load_volumetric(json::Value const& volumetric_value,
                                    Resources&         resources) noexcept {
    Sampler_settings sampler_settings(Sampler_settings::Filter::Linear,
                                      Sampler_settings::Address::Clamp,
                                      Sampler_settings::Address::Clamp);

    Texture_adapter density_map;
    Texture_adapter color_map;
    Texture_adapter emission_map;

    float3 color(0.6f, 0.6f, 0.6f);
    bool   use_absorption_color = false;
    float3 absorption_color(0.f);
    bool   use_scattering_color = false;
    float3 scattering_color(0.f);

    float3 emission(0.f);

    float attenuation_distance = 1.f;
    float anisotropy           = 0.f;
    float a                    = 0.f;
    float b                    = 0.f;

    for (auto& n : volumetric_value.GetObject()) {
        if ("color" == n.name) {
            color = read_color(n.value);
        } else if ("absorption_color" == n.name) {
            use_absorption_color = true;
            absorption_color     = read_color(n.value);
        } else if ("scattering_color" == n.name) {
            use_scattering_color = true;
            scattering_color     = read_color(n.value);
        } else if ("attenuation_distance" == n.name) {
            attenuation_distance = json::read_float(n.value);
        } else if ("emission" == n.name) {
            emission = read_color(n.value);
        } else if ("anisotropy" == n.name) {
            anisotropy = json::read_float(n.value);
        } else if ("a" == n.name) {
            a = json::read_float(n.value);
        } else if ("b" == n.name) {
            b = json::read_float(n.value);
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                memory::Variant_map options;
                if ("Density" == texture_description.usage) {
                    options.set("usage", Texture_usage::Mask);
                    density_map = create_texture(texture_description, options, resources);
                } else if ("Color" == texture_description.usage) {
                    options.set("usage", Texture_usage::Color);
                    color_map = create_texture(texture_description, options, resources);
                }

                /*else if ("Emission" == texture_description.usage) {
                    options.set("usage", Texture_usage::Color);
                    emission_map = create_texture(texture_description, options, resources);
                }*/
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    absorption_color = use_absorption_color ? absorption_color : color;
    scattering_color = use_scattering_color ? scattering_color : color;

    using namespace volumetric;

    if (density_map.is_valid()) {
        if (any_greater_zero(emission)) {
            auto material = new Grid_emission(sampler_settings, density_map);
            material->set_attenuation(absorption_color, scattering_color, attenuation_distance);
            material->set_emission(emission);
            material->set_anisotropy(anisotropy);
            return material;
        }
        auto material = new Grid(sampler_settings, density_map);
        material->set_attenuation(absorption_color, scattering_color, attenuation_distance);
        material->set_emission(emission);
        material->set_anisotropy(anisotropy);
        return material;
    }

    if (color_map.is_valid()) {
        auto material = new Grid_color(sampler_settings);
        material->set_color(color_map);
        material->set_attenuation(1.f, attenuation_distance);
        material->set_emission(emission);
        material->set_anisotropy(anisotropy);
        return material;
    }

    /*else if (a > 0.f && b > 0.f) {
            auto material = new volumetric::Height(sampler_settings);
            material->set_attenuation(absorption_color, scattering_color, attenuation_distance);
            material->set_anisotropy(anisotropy);
            material->set_a_b(a, b);
            return material;
    }*/

    auto material = new Homogeneous(sampler_settings);
    material->set_attenuation(absorption_color, scattering_color, attenuation_distance);
    material->set_emission(emission);
    material->set_anisotropy(anisotropy);
    return material;
}

Sampler_settings::Address read_address(json::Value const& address_value) noexcept {
    std::string const address = json::read_string(address_value);

    if ("Clamp" == address) {
        return Sampler_settings::Address::Clamp;
    }

    if ("Repeat" == address) {
        return Sampler_settings::Address::Repeat;
    }

    return Sampler_settings::Address::Undefined;
}

void read_sampler_settings(json::Value const& sampler_value, Sampler_settings& settings) noexcept {
    for (auto& n : sampler_value.GetObject()) {
        if ("filter" == n.name) {
            std::string const filter = json::read_string(n.value);

            if ("Nearest" == filter) {
                settings.filter = Sampler_settings::Filter::Nearest;
            } else if ("Linear" == filter) {
                settings.filter = Sampler_settings::Filter::Linear;
            }
        } else if ("address" == n.name) {
            if (n.value.IsArray()) {
                auto const address_u = read_address(n.value[0]);
                auto const address_v = read_address(n.value[1]);

                settings.address_u = address_u;
                settings.address_v = address_v;
            } else {
                auto const address = read_address(n.value);

                settings.address_u = address;
                settings.address_v = address;
            }
        }
    }
}

Texture_description read_texture_description(json::Value const& texture_value) noexcept {
    Texture_description description;

    for (auto& n : texture_value.GetObject()) {
        if ("file" == n.name) {
            description.filename = json::read_string(n.value);
        } else if ("id" == n.name) {
            description.filename = image::texture::Provider::encode_name(json::read_uint(n.value));
        } else if ("usage" == n.name) {
            description.usage = json::read_string(n.value);
        } else if ("swizzle" == n.name) {
            std::string const swizzle = json::read_string(n.value);
            if ("YXZW" == swizzle) {
                description.swizzle = image::Swizzle::YXZW;
            }
        } else if ("scale" == n.name) {
            description.scale = json::read_float(n.value);
        } else if ("num_elements" == n.name) {
            description.num_elements = json::read_int(n.value);
        }
    }

    return description;
}

Texture_adapter create_texture(Texture_description const& description, memory::Variant_map& options,
                               Resources& resources) noexcept {
    if (description.num_elements > 1) {
        options.set("num_elements", description.num_elements);
    }

    if (image::Swizzle::XYZW != description.swizzle) {
        options.set("swizzle", description.swizzle);
    }

    return Texture_adapter(resources.load<Texture>(description.filename, options).id,
                           description.scale);
}

void read_coating_description(json::Value const& value, Coating_description& description) noexcept {
    if (!value.IsObject()) {
        return;
    }

    for (auto& n : value.GetObject()) {
        if ("color" == n.name) {
            description.color = json::read_float3(n.value);
        } else if ("attenuation_distance" == n.name) {
            description.attenuation_distance = json::read_float(n.value);
        } else if ("ior" == n.name) {
            description.ior = json::read_float(n.value);
        } else if ("roughness" == n.name) {
            description.roughness = json::read_float(n.value);
        } else if ("thickness" == n.name) {
            description.thickness = json::read_float(n.value);
        } else if ("unit" == n.name) {
            description.in_nm = ("nm" == json::read_string(n.value));
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const texture_description = read_texture_description(tn);

                if (texture_description.filename.empty()) {
                    continue;
                }

                if ("Normal" == texture_description.usage) {
                    description.normal_map_description = texture_description;
                } else if ("Mask" == texture_description.usage) {
                    description.thickness_map_description = texture_description;
                }
            }
        }
    }
}

float3 read_hex_RGB(std::string const& text) noexcept {
    if (7 != text.length() || '#' != text[0]) {
        return float3(0.f);
    }

    static int32_t constexpr hex_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,
        4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15, 0,  0,  0,  0,  0,  0,  0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  10, 11, 12, 13, 14, 15, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0};

    int32_t elements[3];

    char const* buffer = &text.data()[1];
    for (uint32_t i = 0, j = 0; i < 3; ++i, j += 2) {
        elements[i] = (hex_table[uint32_t(buffer[j])] << 4) + hex_table[uint32_t(buffer[j + 1])];
    }

    return float3(float(elements[0]) / 255.f, float(elements[1]) / 255.f,
                  float(elements[2]) / 255.f);
}

float3 read_color(json::Value const& color_value) noexcept {
    if (color_value.IsArray()) {
        return json::read_float3(color_value);
    }

    if (!color_value.IsString()) {
        return float3(0.f);
    }

    std::string const hex_string = json::read_string(color_value);
    return read_hex_RGB(hex_string);
}

float3 read_spectrum(json::Value const& spectrum_value) noexcept {
    if (!spectrum_value.IsObject()) {
        return float3(0.f);
    }

    for (auto& n : spectrum_value.GetObject()) {
        if ("sRGB" == n.name) {
            float3 const srgb = read_color(n.value);
            return spectrum::gamma_to_linear_sRGB(srgb);
        }

        if ("RGB" == n.name) {
            return read_color(n.value);
        }

        if ("temperature" == n.name) {
            float const temperature = json::read_float(n.value);
            return spectrum::blackbody(std::max(800.f, temperature));
        }
    }

    return float3(0.f);
}

uint32_t Provider::max_sample_size() noexcept {
    size_t num_bytes = 0;

    num_bytes = std::max(cloth::Material::sample_size(), num_bytes);
    num_bytes = std::max(display::Constant::sample_size(), num_bytes);
    num_bytes = std::max(glass::Glass::sample_size(), num_bytes);
    num_bytes = std::max(glass::Glass_dispersion::sample_size(), num_bytes);
    num_bytes = std::max(glass::Glass_rough::sample_size(), num_bytes);
    num_bytes = std::max(glass::Glass_thin::sample_size(), num_bytes);
    num_bytes = std::max(light::Constant::sample_size(), num_bytes);
    num_bytes = std::max(matte::Material::sample_size(), num_bytes);
    num_bytes = std::max(metal::Material_anisotropic::sample_size(), num_bytes);
    num_bytes = std::max(metal::Material_isotropic::sample_size(), num_bytes);
    num_bytes = std::max(metallic_paint::Material::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material_clearcoat::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material_coating_subsurface::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material_subsurface::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material_thinfilm::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material_translucent::sample_size(), num_bytes);
    num_bytes = std::max(volumetric::Material::sample_size(), num_bytes);

    num_bytes += num_bytes % 64;

    return uint32_t(num_bytes);
}

}  // namespace scene::material

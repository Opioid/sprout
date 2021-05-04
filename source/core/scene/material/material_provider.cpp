#include "material_provider.hpp"
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "base/memory/array.inl"
#include "base/memory/variant_map.inl"
#include "base/spectrum/aces.hpp"
#include "base/spectrum/mapping.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "debug/debug_material.hpp"
#include "display/display_constant.hpp"
#include "display/display_emissionmap.hpp"
#include "glass/glass_dispersion_material.hpp"
#include "glass/glass_material.hpp"
#include "glass/glass_rough_material.hpp"
#include "glass/glass_thin_material.hpp"
#include "image/channels.hpp"
#include "image/texture/texture.inl"
#include "image/texture/texture_provider.hpp"
#include "light/light_constant.hpp"
#include "light/light_emissionmap.hpp"
#include "logging/logging.hpp"
#include "material_sample_cache.inl"
#include "metal/metal_material.hpp"
#include "metal/metal_presets.hpp"
#include "mix/mix_material.hpp"
#include "null/null_material.hpp"
#include "resource/resource_manager.inl"
#include "resource/resource_provider.inl"
#include "scene/scene_constants.hpp"
#include "substitute/substitute_coating_material.inl"
#include "substitute/substitute_coating_subsurface_material.hpp"
#include "substitute/substitute_material.hpp"
#include "substitute/substitute_subsurface_material.hpp"
#include "substitute/substitute_translucent_material.hpp"
#include "volumetric/volumetric_grid.hpp"
#include "volumetric/volumetric_homogeneous.hpp"

namespace scene::material {

using Texture   = image::texture::Texture;
using Tex_usage = image::texture::Provider::Usage;
using Resources = resource::Manager;
using Variants  = memory::Variant_map;

struct Texture_description {
    std::string filename;

    std::string usage;

    image::Swizzle swizzle = image::Swizzle::Undefined;

    float2 scale = float2(1.f);

    bool invert = false;
};

struct Coating_description {
    float3 color = float3(1.f);

    float attenuation_distance = 0.1f;
    float ior                  = 1.5f;
    float roughness            = 0.2f;
    float thickness            = 0.f;

    bool in_nm = false;

    Texture_description normal_map_description;
    Texture_description thickness_map_description;
};

template <typename Value>
struct Mapped_value {
    Mapped_value(Value v) : value(v){};

    Texture texture;

    Value value;
};

static Texture_description read_texture_description(json::Value const& value, bool no_tex_dwim);

static void read_coating_description(json::Value const& value, bool no_tex_dwim,
                                     Coating_description& description);

static void read_sampler_settings(json::Value const& value, Sampler_settings& settings);

static Texture create_texture(Texture_description const& desc, Tex_usage usage,
                              Resources& resources);

static float3 read_color(json::Value const& value);

static Texture read_texture(json::Value const& value, bool no_tex_dwim, Tex_usage usage,
                            Resources& resources);

static void read_mapped_value(json::Value const& value, bool no_tex_dwim, Tex_usage usage,
                              Resources& resources, Mapped_value<float>& result);

static void read_mapped_value(json::Value const& value, bool no_tex_dwim, Tex_usage usage,
                              Resources& resources, Mapped_value<float3>& result);

Provider::Provider(bool no_tex_dwim, bool force_debug_material)
    : no_tex_dwim_(no_tex_dwim), force_debug_material_(force_debug_material) {
    Material::init_rainbow();
}

Provider::~Provider() = default;

Material* Provider::load(std::string const& filename, Variants const& /*options*/,
                         Resources& resources, std::string& resolved_name) {
    auto stream = resources.filesystem().read_stream(filename, resolved_name);
    if (!stream) {
        return nullptr;
    }

    std::string error;
    auto const  root = json::parse(*stream, error);

    stream.close();

    if (root.HasParseError()) {
        logging::push_error(error);
        return nullptr;
    }

    return load(root, string::parent_directory(resolved_name), resources);
}

Material* Provider::load(void const* data, std::string const&    source_name,
                         Variants const& /*options*/, Resources& resources) {
    json::Value const* value = reinterpret_cast<json::Value const*>(data);

    return load(*value, string::parent_directory(source_name), resources);
}

Material* Provider::create_fallback_material() {
    return new debug::Material(Sampler_settings(Sampler_settings::Filter::Linear));
}

Material* Provider::load(json::Value const& value, std::string_view mount_folder,
                         Resources& resources) const {
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
                material = new debug::Material(Sampler_settings());
            }
        } else {
            if ("Debug" == n.name) {
                material = load_debug(n.value, resources);
            } else if ("Display" == n.name) {
                material = load_display(n.value, resources);
            } else if ("Glass" == n.name) {
                material = load_glass(n.value, resources);
            } else if ("Light" == n.name) {
                material = load_light(n.value, resources);
            } else if ("Metal" == n.name) {
                material = load_metal(n.value, resources);
            } else if ("Mix" == n.name) {
                material = load_mix(n.value, resources);
            } else if ("Null" == n.name) {
                material = new null::Material(Sampler_settings());
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

Material* Provider::load_debug(json::Value const& debug_value, Resources& resources) const {
    Sampler_settings sampler_settings;

    Texture mask;

    for (auto const& n : debug_value.GetObject()) {
        if ("mask" == n.name) {
            mask = read_texture(n.value, no_tex_dwim_, Tex_usage::Mask, resources);
        } else if ("textures" == n.name) {
            for (auto const& tn : n.value.GetArray()) {
                Texture_description const desc = read_texture_description(tn, no_tex_dwim_);

                if (desc.filename.empty()) {
                    continue;
                }

                if ("Mask" == desc.usage) {
                    mask = create_texture(desc, Tex_usage::Mask, resources);
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

Material* Provider::load_display(json::Value const& display_value, Resources& resources) const {
    Sampler_settings sampler_settings;

    Texture mask;
    Texture emission_map;

    bool two_sided = false;

    float3 radiance(10.f);

    float emission_factor = 1.f;
    float roughness       = 1.f;
    float ior             = 1.5;

    for (auto const& n : display_value.GetObject()) {
        if ("mask" == n.name) {
            mask = read_texture(n.value, no_tex_dwim_, Tex_usage::Mask, resources);
        } else if ("radiance" == n.name) {
            radiance = read_color(n.value);
        } else if ("emission_factor" == n.name) {
            emission_factor = json::read_float(n.value);
        } else if ("roughness" == n.name) {
            roughness = json::read_float(n.value);
        } else if ("ior" == n.name) {
            ior = json::read_float(n.value);
        } else if ("two_sided" == n.name) {
            two_sided = json::read_bool(n.value);
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const desc = read_texture_description(tn, no_tex_dwim_);

                if (desc.filename.empty()) {
                    continue;
                }

                if ("Emission" == desc.usage) {
                    emission_map = create_texture(desc, Tex_usage::Color, resources);
                } else if ("Mask" == desc.usage) {
                    mask = create_texture(desc, Tex_usage::Mask, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    if (emission_map.is_valid()) {
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

Material* Provider::load_glass(json::Value const& glass_value, Resources& resources) const {
    Sampler_settings sampler_settings;

    Mapped_value<float> roughness(0.f);

    Texture mask;
    Texture normal_map;

    float3 refraction_color(1.f);
    float3 attenuation_color(1.f);

    float attenuation_distance = 1.f;
    float ior                  = 1.5f;
    float abbe                 = 0.f;
    float thickness            = 0.f;

    for (auto const& n : glass_value.GetObject()) {
        if ("mask" == n.name) {
            mask = read_texture(n.value, no_tex_dwim_, Tex_usage::Mask, resources);
        } else if ("normal" == n.name) {
            normal_map = read_texture(n.value, no_tex_dwim_, Tex_usage::Normal, resources);
        } else if ("color" == n.name || "attenuation_color" == n.name) {
            attenuation_color = read_color(n.value);
        } else if ("refraction_color" == n.name) {
            refraction_color = read_color(n.value);
        } else if ("attenuation_distance" == n.name) {
            attenuation_distance = json::read_float(n.value);
        } else if ("ior" == n.name) {
            ior = json::read_float(n.value);
        } else if ("abbe" == n.name) {
            abbe = json::read_float(n.value);
        } else if ("roughness" == n.name) {
            read_mapped_value(n.value, no_tex_dwim_, Tex_usage::Roughness, resources, roughness);
        } else if ("thickness" == n.name) {
            thickness = json::read_float(n.value);
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const desc = read_texture_description(tn, no_tex_dwim_);

                if (desc.filename.empty()) {
                    continue;
                }

                if ("Mask" == desc.usage) {
                    mask = create_texture(desc, Tex_usage::Mask, resources);
                } else if ("Normal" == desc.usage) {
                    normal_map = create_texture(desc, Tex_usage::Normal, resources);
                } else if ("Roughness" == desc.usage) {
                    roughness.texture = create_texture(desc, Tex_usage::Roughness, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    if (roughness.value > 0.f || roughness.texture.is_valid()) {
        auto material = new glass::Glass_rough(sampler_settings);
        material->set_normal_map(normal_map);
        material->set_roughness_map(roughness.texture);
        material->set_refraction_color(refraction_color);
        material->set_volumetric(attenuation_color, float3(0.f), attenuation_distance, 0.f);
        material->set_ior(ior);
        material->set_roughness(roughness.value);
        return material;
    }

    if (thickness > 0.f) {
        auto material = new glass::Glass_thin(sampler_settings);
        material->set_mask(mask);
        material->set_normal_map(normal_map);
        material->set_refraction_color(refraction_color);
        material->set_volumetric(attenuation_color, float3(0.f), attenuation_distance, 0.f);
        material->set_ior(ior);
        material->set_thickness(thickness);
        return material;
    }

    if (abbe > 0.f) {
        auto material = new glass::Glass_dispersion(sampler_settings);
        material->set_normal_map(normal_map);
        material->set_refraction_color(refraction_color);
        material->set_volumetric(attenuation_color, float3(0.f), attenuation_distance, 0.f);
        material->set_ior(ior);
        material->set_abbe(abbe);
        return material;
    }

    auto material = new glass::Glass(sampler_settings);
    material->set_normal_map(normal_map);
    material->set_refraction_color(refraction_color);
    material->set_volumetric(attenuation_color, float3(0.f), attenuation_distance, 0.f);
    material->set_ior(ior);
    return material;
}

Material* Provider::load_light(json::Value const& light_value, Resources& resources) const {
    Sampler_settings sampler_settings;

    std::string quantity;

    Mapped_value<float3> emission(float3(10.f));

    float3 color(1.f);

    float value           = 1.f;
    float emission_factor = 1.f;

    Texture mask;

    bool two_sided = false;

    for (auto const& n : light_value.GetObject()) {
        if ("mask" == n.name) {
            mask = read_texture(n.value, no_tex_dwim_, Tex_usage::Mask, resources);
        } else if ("emission" == n.name) {
            read_mapped_value(n.value, no_tex_dwim_, Tex_usage::Color, resources, emission);
        } else if ("emittance" == n.name) {
            quantity = json::read_string(n.value, "quantity");

            auto const s = n.value.FindMember("spectrum");
            if (n.value.MemberEnd() != s) {
                color = read_color(s->value);
            }

            value = json::read_float(n.value, "value", value);
        } else if ("emission_factor" == n.name) {
            emission_factor = json::read_float(n.value);
        } else if ("two_sided" == n.name) {
            two_sided = json::read_bool(n.value);
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const desc = read_texture_description(tn, no_tex_dwim_);

                if (desc.filename.empty()) {
                    continue;
                }

                if ("Emission" == desc.usage) {
                    emission.texture = create_texture(desc, Tex_usage::Color, resources);
                } else if ("Mask" == desc.usage) {
                    mask = create_texture(desc, Tex_usage::Mask, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    if (emission.texture.is_valid()) {
        auto material = new light::Emissionmap(sampler_settings, two_sided);
        material->set_mask(mask);
        material->set_emission_map(emission.texture);
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
        material->emittance().set_radiance(emission.value);
    }

    return material;
}

Material* Provider::load_metal(json::Value const& metal_value, Resources& resources) const {
    Sampler_settings sampler_settings;

    Texture normal_map;
    //	Texture_ptr surface_map;
    Texture direction_map;
    Texture mask;

    bool two_sided = false;

    float3 ior(1.f, 1.f, 1.f);
    float3 absorption(0.75f, 0.75f, 0.75f);
    float  roughness = 0.9f;
    float2 roughness_aniso(0.f, 0.f);

    for (auto const& n : metal_value.GetObject()) {
        if ("mask" == n.name) {
            mask = read_texture(n.value, no_tex_dwim_, Tex_usage::Mask, resources);
        } else if ("normal" == n.name) {
            normal_map = read_texture(n.value, no_tex_dwim_, Tex_usage::Normal, resources);
        } else if ("ior" == n.name) {
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
                Texture_description const desc = read_texture_description(tn, no_tex_dwim_);

                if (desc.filename.empty()) {
                    continue;
                }

                if ("Normal" == desc.usage) {
                    normal_map = create_texture(desc, Tex_usage::Normal, resources);
                } else if ("Anisotropy" == desc.usage) {
                    direction_map = create_texture(desc, Tex_usage::Anisotropy, resources);
                } else if ("Mask" == desc.usage) {
                    mask = create_texture(desc, Tex_usage::Mask, resources);
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

Material* Provider::load_mix(json::Value const& mix_value, Resources& resources) const {
    Sampler_settings sampler_settings;

    Texture mask;

    bool two_sided = false;

    memory::Array<material::Material*> materials;
    materials.reserve(2);

    for (auto const& n : mix_value.GetObject()) {
        if ("mask" == n.name) {
            mask = read_texture(n.value, no_tex_dwim_, Tex_usage::Mask, resources);
        } else if ("materials" == n.name) {
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
                Texture_description const desc = read_texture_description(tn, no_tex_dwim_);

                if (desc.filename.empty()) {
                    continue;
                }

                if ("Mask" == desc.usage) {
                    mask = create_texture(desc, Tex_usage::Mask, resources);
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

Material* Provider::load_substitute(json::Value const& value, Resources& resources) const {
    Sampler_settings sampler_settings;

    Mapped_value<float3> color(float3(0.5f));
    Mapped_value<float3> emission(float3(0.f));
    Mapped_value<float>  roughness(0.8f);

    Texture normal_map;
    Texture mask;
    Texture density_map;

    bool two_sided = false;

    float3 attenuation_color(0.f);
    float3 subsurface_color(0.f);

    float3 checkers[2];
    float  checkers_scale = 0.f;

    float metallic              = 0.f;
    float ior                   = 1.46f;
    float emission_factor       = 1.f;
    float thickness             = 0.f;
    float attenuation_distance  = 0.f;
    float volumetric_anisotropy = 0.f;

    Coating_description coating;

    for (auto const& n : value.GetObject()) {
        if ("mask" == n.name) {
            mask = read_texture(n.value, no_tex_dwim_, Tex_usage::Mask, resources);
        } else if ("normal" == n.name) {
            normal_map = read_texture(n.value, no_tex_dwim_, Tex_usage::Normal, resources);
        } else if ("color" == n.name) {
            read_mapped_value(n.value, no_tex_dwim_, Tex_usage::Color, resources, color);
        } else if ("emission" == n.name) {
            read_mapped_value(n.value, no_tex_dwim_, Tex_usage::Color, resources, emission);
        } else if ("surface" == n.name) {
            roughness.texture = read_texture(n.value, no_tex_dwim_, Tex_usage::Surface, resources);
        } else if ("checkers" == n.name) {
            for (auto const& cn : n.value.GetObject()) {
                if ("scale" == cn.name) {
                    checkers_scale = json::read_float(cn.value);
                } else if ("colors" == cn.name) {
                    auto const ca = cn.value.GetArray();

                    checkers[0] = read_color(ca[0]);
                    checkers[1] = read_color(ca[1]);
                }
            }
        } else if ("metal_preset" == n.name) {
            float3 eta;
            float3 k;
            metal::ior_and_absorption(n.value.GetString(), eta, k);

            color = fresnel::conductor(1.f, eta, k);

            metallic = 1.f;
        } else if ("attenuation_color" == n.name) {
            attenuation_color = read_color(n.value);
        } else if ("subsurface_color" == n.name) {
            subsurface_color = read_color(n.value);
        } else if ("ior" == n.name) {
            ior = json::read_float(n.value);
        } else if ("roughness" == n.name) {
            read_mapped_value(n.value, no_tex_dwim_, Tex_usage::Roughness, resources, roughness);
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
            read_coating_description(n.value, no_tex_dwim_, coating);
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const desc = read_texture_description(tn, no_tex_dwim_);

                if (desc.filename.empty()) {
                    continue;
                }

                if ("Color" == desc.usage) {
                    color.texture = create_texture(desc, Tex_usage::Color, resources);
                } else if ("Normal" == desc.usage) {
                    normal_map = create_texture(desc, Tex_usage::Normal, resources);
                } else if ("Surface" == desc.usage) {
                    roughness.texture = create_texture(desc, Tex_usage::Surface, resources);
                } else if ("Roughness" == desc.usage) {
                    roughness.texture = create_texture(desc, Tex_usage::Roughness, resources);
                } else if ("Emission" == desc.usage) {
                    emission.texture = create_texture(desc, Tex_usage::Color, resources);
                } else if ("Mask" == desc.usage) {
                    mask = create_texture(desc, Tex_usage::Mask, resources);
                } else if ("Density" == desc.usage) {
                    density_map = create_texture(desc, Tex_usage::Mask, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    if (thickness > 0.f) {
        auto material = new substitute::Material_translucent(sampler_settings);

        material->set_mask(mask);
        material->set_color_map(color.texture);
        material->set_normal_map(normal_map);
        material->set_surface_map(roughness.texture);
        material->set_emission_map(emission.texture);

        material->set_color(color.value);
        material->set_emission(emission.value);
        material->set_ior(ior);
        material->set_roughness(roughness.value);
        material->set_metallic(metallic);
        material->set_emission_factor(emission_factor);
        material->set_volumetric(thickness, attenuation_distance);

        return material;
    }

    if (coating.thickness > 0.f) {
        Texture coating_thickness_map;
        Texture coating_normal_map;

        if (!coating.thickness_map_description.filename.empty()) {
            coating_thickness_map = create_texture(coating.thickness_map_description,
                                                   Tex_usage::Mask, resources);
        }

        if (!coating.normal_map_description.filename.empty()) {
            coating_normal_map = create_texture(coating.normal_map_description, Tex_usage::Normal,
                                                resources);
        }

        if (coating.in_nm) {
            auto material = new substitute::Material_thinfilm(sampler_settings, two_sided);

            material->set_mask(mask);
            material->set_color_map(color.texture);
            material->set_normal_map(normal_map);
            material->set_surface_map(roughness.texture);
            material->set_emission_map(emission.texture);

            material->set_color(color.value);
            material->set_emission(emission.value);
            material->set_ior(ior);
            material->set_roughness(roughness.value);
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
            material->set_color_map(color.texture);
            material->set_normal_map(normal_map);
            material->set_surface_map(roughness.texture);
            material->set_emission_map(emission.texture);
            material->set_density_map(density_map);

            material->set_color(color.value);
            material->set_emission(emission.value);
            material->set_volumetric(attenuation_color, subsurface_color, attenuation_distance,
                                     volumetric_anisotropy);
            material->set_ior(ior);
            material->set_roughness(roughness.value);
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
        material->set_color_map(color.texture);
        material->set_normal_map(normal_map);
        material->set_surface_map(roughness.texture);
        material->set_emission_map(emission.texture);

        material->set_color(color.value);
        material->set_emission(emission.value);
        material->set_ior(ior);
        material->set_roughness(roughness.value);
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

    if (!two_sided && (attenuation_distance > 0.f || density_map.is_valid())) {
        auto material = new substitute::Material_subsurface(sampler_settings);

        material->set_mask(mask);
        material->set_color_map(color.texture);
        material->set_normal_map(normal_map);
        material->set_surface_map(roughness.texture);
        material->set_emission_map(emission.texture);
        material->set_density_map(density_map);

        material->set_color(color.value);
        material->set_emission(emission.value);
        material->set_volumetric(attenuation_color, subsurface_color, attenuation_distance,
                                 volumetric_anisotropy);
        material->set_ior(ior);
        material->set_roughness(roughness.value);
        material->set_metallic(metallic);
        material->set_emission_factor(emission_factor);

        return material;
    }

    if (checkers_scale > 0.f) {
        auto material = new substitute::Checkers(sampler_settings, two_sided);

        material->set_mask(mask);
        material->set_normal_map(normal_map);
        material->set_surface_map(roughness.texture);
        material->set_emission_map(emission.texture);

        material->set_checkers(checkers[0], checkers[1], checkers_scale);
        material->set_emission(emission.value);
        material->set_ior(ior);
        material->set_roughness(roughness.value);
        material->set_metallic(metallic);
        material->set_emission_factor(emission_factor);

        return material;
    }

    auto material = new substitute::Material(sampler_settings, two_sided);

    material->set_mask(mask);
    material->set_color_map(color.texture);
    material->set_normal_map(normal_map);
    material->set_surface_map(roughness.texture);
    material->set_emission_map(emission.texture);

    material->set_color(color.value);
    material->set_emission(emission.value);
    material->set_ior(ior);
    material->set_roughness(roughness.value);
    material->set_metallic(metallic);
    material->set_emission_factor(emission_factor);

    return material;
}

Material* Provider::load_volumetric(json::Value const& value, Resources& resources) const {
    Sampler_settings sampler_settings(Sampler_settings::Filter::Linear,
                                      Sampler_settings::Address::Clamp,
                                      Sampler_settings::Address::Clamp);

    Mapped_value<float3> color(float3(0.5f));

    Texture density_map;
    Texture temperature_map;

    float3 attenuation_color(0.f);
    float3 subsurface_color(0.f);
    float3 emission(0.f);

    bool use_attenuation_color = false;
    bool use_subsurface_color  = false;

    float attenuation_distance = 1.f;
    float anisotropy           = 0.f;
    float a                    = 0.f;
    float b                    = 0.f;

    for (auto& n : value.GetObject()) {
        if ("density" == n.name) {
            density_map = read_texture(n.value, no_tex_dwim_, Tex_usage::Mask, resources);
        } else if ("color" == n.name) {
            read_mapped_value(n.value, no_tex_dwim_, Tex_usage::Color, resources, color);
        } else if ("attenuation_color" == n.name) {
            use_attenuation_color = true;
            attenuation_color     = read_color(n.value);
        } else if ("subsurface_color" == n.name) {
            use_subsurface_color = true;
            subsurface_color     = read_color(n.value);
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
                Texture_description const desc = read_texture_description(tn, no_tex_dwim_);

                if (desc.filename.empty()) {
                    continue;
                }

                if ("Density" == desc.usage) {
                    density_map = create_texture(desc, Tex_usage::Mask, resources);
                } else if ("Color" == desc.usage) {
                    color.texture = create_texture(desc, Tex_usage::Color, resources);
                } else if ("Temperature" == desc.usage) {
                    temperature_map = create_texture(desc, Tex_usage::Color, resources);
                }
            }
        } else if ("sampler" == n.name) {
            read_sampler_settings(n.value, sampler_settings);
        }
    }

    attenuation_color = use_attenuation_color ? attenuation_color : color.value;
    subsurface_color  = use_subsurface_color ? subsurface_color : color.value;

    using namespace volumetric;

    if (density_map.is_valid()) {
        if (any_greater_zero(emission) || temperature_map.is_valid()) {
            auto material = new Grid_emission(sampler_settings, density_map);
            material->set_volumetric(attenuation_color, subsurface_color, attenuation_distance,
                                     anisotropy);
            material->set_emission(emission);
            material->set_temperature_map(temperature_map);
            return material;
        }

        auto material = new Grid(sampler_settings, density_map);
        material->set_volumetric(attenuation_color, subsurface_color, attenuation_distance,
                                 anisotropy);
        material->set_emission(emission);
        return material;
    }

    if (color.texture.is_valid()) {
        auto material = new Grid_color(sampler_settings);
        material->set_color(color.texture);
        material->set_volumetric(1.f, attenuation_distance, anisotropy);
        material->set_emission(emission);
        return material;
    }

    auto material = new Homogeneous(sampler_settings);
    material->set_volumetric(attenuation_color, subsurface_color, attenuation_distance, anisotropy);
    material->set_emission(emission);
    return material;
}

Sampler_settings::Address read_address(json::Value const& address_value) {
    std::string const address = json::read_string(address_value);

    if ("Clamp" == address) {
        return Sampler_settings::Address::Clamp;
    }

    if ("Repeat" == address) {
        return Sampler_settings::Address::Repeat;
    }

    return Sampler_settings::Address::Undefined;
}

void read_sampler_settings(json::Value const& value, Sampler_settings& settings) {
    for (auto& n : value.GetObject()) {
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

Texture_description read_texture_description(json::Value const& value, bool no_tex_dwim) {
    using image::Swizzle;

    Texture_description desc;

    for (auto& n : value.GetObject()) {
        if ("file" == n.name) {
            desc.filename = json::read_string(n.value);
        } else if ("id" == n.name) {
            desc.filename = image::texture::Provider::encode_name(json::read_uint(n.value));
        } else if ("usage" == n.name) {
            desc.usage = json::read_string(n.value);
        } else if ("swizzle" == n.name) {
            std::string const swizzle = json::read_string(n.value);

            if ("X" == swizzle) {
                desc.swizzle = Swizzle::X;
            } else if ("Y" == swizzle) {
                desc.swizzle = Swizzle::Y;
            } else if ("Z" == swizzle) {
                desc.swizzle = Swizzle::Z;
            } else if ("W" == swizzle) {
                desc.swizzle = Swizzle::W;
            } else if ("XY" == swizzle) {
                desc.swizzle = Swizzle::XY;
            } else if ("YX" == swizzle) {
                desc.swizzle = Swizzle::YX;
            } else if ("XYZ" == swizzle) {
                desc.swizzle = Swizzle::XYZ;
            } else if ("YXZ" == swizzle) {
                desc.swizzle = Swizzle::YXZ;
            } else if ("XYZW" == swizzle) {
                desc.swizzle = Swizzle::XYZW;
            } else if ("YXZW" == swizzle) {
                desc.swizzle = Swizzle::YXZW;
            }
        } else if ("scale" == n.name) {
            if (n.value.IsArray()) {
                desc.scale = json::read_float2(n.value);
            } else {
                float const s = json::read_float(n.value);
                desc.scale    = float2(s, s);
            }
        } else if ("invert" == n.name) {
            desc.invert = json::read_bool(n.value);
        }
    }

    if (no_tex_dwim && "Emission" != desc.usage) {
        desc.filename.clear();
    }

    return desc;
}

Texture create_texture(Texture_description const& desc, Tex_usage usage, Resources& resources) {
    Variants options;
    options.set("usage", usage);

    if (image::Swizzle::Undefined != desc.swizzle) {
        options.set("swizzle", desc.swizzle);
    }

    if (desc.invert) {
        options.set("invert", desc.invert);
    }

    return image::texture::Provider::load(desc.filename, options, desc.scale, resources);
}

void read_coating_description(json::Value const& value, bool no_tex_dwim,
                              Coating_description& coating) {
    if (!value.IsObject()) {
        return;
    }

    for (auto& n : value.GetObject()) {
        if ("color" == n.name) {
            coating.color = json::read_float3(n.value);
        } else if ("attenuation_distance" == n.name) {
            coating.attenuation_distance = json::read_float(n.value);
        } else if ("ior" == n.name) {
            coating.ior = json::read_float(n.value);
        } else if ("roughness" == n.name) {
            coating.roughness = json::read_float(n.value);
        } else if ("thickness" == n.name) {
            coating.thickness = json::read_float(n.value);
        } else if ("unit" == n.name) {
            coating.in_nm = ("nm" == json::read_string(n.value));
        } else if ("textures" == n.name) {
            for (auto& tn : n.value.GetArray()) {
                Texture_description const desc = read_texture_description(tn, no_tex_dwim);

                if (desc.filename.empty()) {
                    continue;
                }

                if ("Normal" == desc.usage) {
                    coating.normal_map_description = desc;
                } else if ("Mask" == desc.usage) {
                    coating.thickness_map_description = desc;
                }
            }
        }
    }
}

static float3 read_hex_RGB(std::string const& text) {
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

static inline float3 map_color(float3_p color) {
#ifdef SU_ACESCG
    return spectrum::sRGB_to_AP1(color);
#else
    return color;
#endif
}

float3 read_color(json::Value const& value) {
    if (value.IsNumber()) {
        return float3(map_color(float3(json::read_float(value)))[1]);
    }

    if (value.IsArray()) {
        return map_color(json::read_float3(value));
    }

    if (value.IsObject()) {
        float3 rgb(0.f);

        bool linear = true;

        for (auto& n : value.GetObject()) {
            if ("sRGB" == n.name) {
                rgb = read_color(n.value);
            }

            if ("temperature" == n.name) {
                float const temperature = json::read_float(n.value);

                rgb = spectrum::blackbody(std::max(800.f, temperature));
            }

            if ("linear" == n.name) {
                linear = json::read_bool(n.value);
            }
        }

        if (!linear) {
            rgb = spectrum::gamma_to_linear_sRGB(rgb);
        }

        return map_color(rgb);
    }

    if (!value.IsString()) {
        return float3(0.f);
    }

    std::string const hex_string = json::read_string(value);
    return map_color(read_hex_RGB(hex_string));
}

Texture read_texture(json::Value const& value, bool no_tex_dwim, Tex_usage usage,
                     Resources& resources) {
    Texture_description const desc = read_texture_description(value, no_tex_dwim);

    if (!desc.filename.empty()) {
        return create_texture(desc, usage, resources);
    }

    return Texture();
}

void read_mapped_value(json::Value const& value, bool no_tex_dwim, Tex_usage usage,
                       Resources& resources, Mapped_value<float>& result) {
    if (!value.IsObject() || value.MemberEnd() == value.FindMember("file")) {
        result.value = json::read_float(value);
    } else {
        Texture_description const desc = read_texture_description(value, no_tex_dwim);

        if (!desc.filename.empty()) {
            result.texture = create_texture(desc, usage, resources);
        }
    }
}

void read_mapped_value(json::Value const& value, bool no_tex_dwim, Tex_usage usage,
                       Resources& resources, Mapped_value<float3>& result) {
    if (!value.IsObject() || value.MemberEnd() == value.FindMember("file")) {
        result.value = read_color(value);
    } else {
        Texture_description const desc = read_texture_description(value, no_tex_dwim);

        if (!desc.filename.empty()) {
            result.texture = create_texture(desc, usage, resources);
        }
    }
}

uint32_t Provider::max_sample_size() {
    size_t num_bytes = 0;

    num_bytes = std::max(display::Constant::sample_size(), num_bytes);
    num_bytes = std::max(glass::Glass::sample_size(), num_bytes);
    num_bytes = std::max(glass::Glass_dispersion::sample_size(), num_bytes);
    num_bytes = std::max(glass::Glass_rough::sample_size(), num_bytes);
    num_bytes = std::max(glass::Glass_thin::sample_size(), num_bytes);
    num_bytes = std::max(light::Constant::sample_size(), num_bytes);
    num_bytes = std::max(metal::Material_anisotropic::sample_size(), num_bytes);
    num_bytes = std::max(metal::Material_isotropic::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material_clearcoat::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material_coating_subsurface::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material_subsurface::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material_thinfilm::sample_size(), num_bytes);
    num_bytes = std::max(substitute::Material_translucent::sample_size(), num_bytes);
    num_bytes = std::max(volumetric::Material::sample_size(), num_bytes);

    size_t const r = num_bytes % 64;

    num_bytes += 0 == r ? 0 : 64 - r;

    return uint32_t(num_bytes);
}

}  // namespace scene::material

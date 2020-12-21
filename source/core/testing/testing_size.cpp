#include "testing_size.hpp"
#include "base/math/vector3.inl"
#include "image/image.hpp"
#include "image/texture/texture.hpp"
#include "image/texture/texture_adapter.hpp"
#include "image/texture/texture_byte3_srgb.hpp"
#include "image/typed_image.hpp"
#include "rendering/integrator/particle/photon/photon.hpp"
#include "rendering/integrator/surface/pathtracer_mis.hpp"
#include "rendering/rendering_worker.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "scene/bvh/scene_bvh_builder.hpp"
#include "scene/bvh/scene_bvh_builder_base.hpp"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/bvh/scene_bvh_split_candidate.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/entity/keyframe.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/debug/debug_material.hpp"
#include "scene/material/debug/debug_sample.hpp"
#include "scene/material/glass/glass_material.hpp"
#include "scene/material/glass/glass_sample.hpp"
#include "scene/material/light/light_constant.hpp"
#include "scene/material/light/light_emissionmap.hpp"
#include "scene/material/light/light_material_sample.hpp"
#include "scene/material/metal/metal_material.hpp"
#include "scene/material/metal/metal_sample.hpp"
#include "scene/material/substitute/substitute_material.hpp"
#include "scene/material/substitute/substitute_sample.hpp"
#include "scene/material/volumetric/volumetric_grid.hpp"
#include "scene/material/volumetric/volumetric_octree.hpp"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/shape/canopy.hpp"
#include "scene/shape/distant_sphere.hpp"
#include "scene/shape/infinite_sphere.hpp"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/sphere.hpp"
#include "scene/shape/triangle/bvh/triangle_bvh_indexed_data.hpp"
#include "scene/shape/triangle/triangle_mesh.hpp"
#include "scene/shape/triangle/triangle_morphable_mesh.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.hpp"

#include <iostream>
#include <string>

namespace testing {

template <typename T>
void print_size(std::string const& name, size_t expected_size) {
    if (sizeof(T) != expected_size) {
        std::cout << "ALARM: sizeof(" << name << ") == " << sizeof(T) << " (" << expected_size
                  << ")" << std::endl;
    } else {
        std::cout << "sizeof(" << name << ") == " << sizeof(T) << std::endl;
    }
}

void size() {
    std::cout << "Sizes:" << std::endl;

    print_size<float3>("float3", 16);

    print_size<float4>("float4", 16);

    print_size<math::Matrix3x3f_a>("float3x3", 48);

    print_size<float4x4>("float4x4", 64);

    print_size<math::Transformation>("transformation", 48);

    print_size<scene::entity::Keyframe>("Keyframe", 48);

    print_size<scene::entity::Composed_transformation>("Composed_transformation", 128);

    print_size<ray>("ray", 48);
    print_size<scene::Ray>("scene::Ray", 64);

    print_size<rendering::integrator::particle::photon::Photon>("Photon", 48);

    print_size<sampler::Sampler>("Sampler", 8);
    print_size<sampler::Random>("Random", 8);
    print_size<sampler::Golden_ratio>("Golden_ratio", 64);

    print_size<scene::shape::Intersection>("shape::Intersection", 96);

    print_size<scene::prop::Intersection>("prop::Intersection", 112);

    print_size<scene::material::Material>("material::Material", 96);
    print_size<scene::material::bxdf::Sample>("bxdf::Sample", 64);

    print_size<scene::material::debug::Material>("debug::Material", 96);
    print_size<scene::material::debug::Sample>("debug::Sample", 160);

    print_size<scene::material::glass::Glass>("glass::Glass", 128);
    print_size<scene::material::glass::Sample>("glass::Sample", 160);

    print_size<scene::material::light::Constant>("light::Constant", 128);
    print_size<scene::material::light::Emissionmap>("light::Emissionmap", 176);
    print_size<scene::material::light::Sample>("light::Sample", 160);

    print_size<scene::material::metal::Material_isotropic>("metal::Material_isotropic", 160);
    print_size<scene::material::metal::Material_anisotropic>("metal::Material_anisotropic", 160);
    print_size<scene::material::metal::Sample_isotropic>("metal::Sample_isotropic", 192);
    print_size<scene::material::metal::Sample_anisotropic>("metal::Sample_anisotropic", 208);

    print_size<scene::material::substitute::Material>("substitute::Material", 176);
    print_size<scene::material::substitute::Sample>("substitute::Sample", 208);

    print_size<scene::material::volumetric::Gridtree>("volumetric::Gridtree", 64);
    print_size<scene::material::volumetric::Grid>("volumetric::Grid", 192);

    print_size<scene::prop::Prop>("prop::Prop", 8);

    print_size<scene::Renderstate>("Renderstate", 112);

    print_size<scene::bvh::Node>("scene::bvh::Node", 32);

    print_size<float3>("Intersection_vertex_MT", 16);

    print_size<scene::shape::Canopy>("Canopy", 16);
    print_size<scene::shape::Distant_sphere>("Distant_sphere", 16);
    print_size<scene::shape::Sphere>("Sphere", 16);
    print_size<scene::shape::Infinite_sphere>("Infinite_sphere", 16);
    print_size<scene::shape::triangle::Mesh>("Mesh", 128);
    print_size<scene::shape::triangle::Morphable_mesh>("Morphable_mesh", 128);

    print_size<scene::shape::triangle::bvh::Indexed_data::Index_triangle>("Index_triangle", 16);

    //	print_size<scene::shape::triangle::Shading_vertex_MT>("Shading_vertex_MT", 48);

    print_size<scene::shape::triangle::Shading_vertex_MTC>("Shading_vertex_MTC", 32);

    print_size<scene::shape::Vertex>("Vertex", 48);

    print_size<image::Image>("image::Image", 64);

    print_size<image::Byte3>("image::Byte3", 48);

    print_size<image::texture::Texture>("texture::Texture", 48);

    print_size<image::texture::Byte3_sRGB>("texture::Byte_sRGB", 32);

    print_size<image::texture::Adapter>("texture::Adapter", 8);

    print_size<scene::bvh::Split_candidate>("Split_candidate", 96);

    print_size<scene::shape::Node_stack>("scene::shape::Node_stack", 512);

    print_size<scene::Worker>("scene::Worker", 960);
    print_size<rendering::Worker>("rendering::Worker", 1024);

    print_size<rendering::integrator::surface::Pathtracer_MIS>("PTMIS", 128);
}

}  // namespace testing

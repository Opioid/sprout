#include "glass_test.hpp"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "glass_rough_sample.hpp"
#include "glass_sample.hpp"
#include "sampler/sampler_random.hpp"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_print.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::glass::testing {

void test() {
    /*
    rnd::Generator  rng;
    sampler::Random sampler(rng);
    sampler.resize(0, 1, 1, 1);

    bxdf::Sample result;

    Sample sample;

    float3 t(1.f, 0.f, 0.f);
    float3 b(0.f, 1.f, 0.f);
    float3 n(0.f, 0.f, 1.f);

    float3 wo;

    wo = normalize(float3(0.25f, 0.25f, -0.5f));
    ;
    sample.set_basis(n, wo);
    sample.layer_.set_tangent_frame(t, b, n);

    float3 refraction_color(1.f, 1.f, 1.f);
    float3 absorption_color(1.f, 1.f, 1.f);
    float  attenuation_distance = 1.f;
    float  ior                  = 1.5f;

    sample.layer_.set(refraction_color, absorption_color, attenuation_distance, ior, 1.f);

    result.wi         = float3::identity();
    result.reflection = float3::identity();
    Sample::BSDF::reflect(sample, sample.layer_, result);
    print(result);
    result.reflection = float3::identity();
    Sample::BSDF::refract(sample, sample.layer_, result);
    print(result);

    wo = b;
    sample.set_basis(n, wo);
    result.reflection = float3::identity();
    Sample::BSDF::reflect(sample, sample.layer_, result);
    print(result);
    result.reflection = float3::identity();
    Sample::BSDF::refract(sample, sample.layer_, result);
    print(result);

    wo = n;
    sample.set_basis(n, wo);
    result.reflection = float3::identity();
    Sample::BSDF::reflect(sample, sample.layer_, result);
    print(result);
    result.reflection = float3::identity();
    Sample::BSDF::refract(sample, sample.layer_, result);
    print(result);
    */
}

void rough_refraction() {
    rnd::Generator rng(0, 0);

    sampler::Random sampler(rng);
    sampler.resize(0, 1, 1, 1);

    Sample sample;

    Sample_rough sample_rough;

    float3 const t(-1.f, 0.f, 0.f);
    float3 const b(0.f, -1.f, 0.f);
    float3 const n(0.f, 0.f, -1.f);

    //  float3 const geo_n(0.f, 0.f, 1.f);

    //  n = normalize(float3(0.5f, 0.5f, 0.5f));
    //  orthonormal_basis(n, t, b);

    float3 wo = normalize(float3(0.2f, -0.2f, 0.9f));

    sample.set_basis(n, wo);
    sample.layer_.set_tangent_frame(t, b, n);

    sample_rough.set_basis(n, wo);
    sample_rough.layer_.set_tangent_frame(t, b, n);

    float3 refraction_color(1.f, 1.f, 1.f);
    float3 absorption_color(1.f, 1.f, 1.f);

    float attenuation_distance = 1.f;
    float ior                  = 1.02f;
    float roughness            = 0.5f;  // ggx::Min_roughness;
    float alpha                = roughness * roughness;

    sample.set(refraction_color, ior, 1.f);

    sample_rough.set(refraction_color, ior, 1.f, alpha, false);

    bool const same_side = sample.same_hemisphere(wo);

    //   sample.refract(same_side, sample.layer_, sampler, result);

    std::cout << "wo: " << wo << std::endl;

    //    {
    //        std::cout << "perfect:" << std::endl;

    //        bxdf::Sample result;

    //        result.wi         = float3::identity();
    //        result.reflection = float3::identity();

    //        sample.sample(sampler, result);

    //        std::cout << "sample:" << std::endl;
    //        print(result);

    //        if (result.pdf > 0.f) {
    //            std::cout << "\nevaluate:" << std::endl;
    //            bxdf::Result eval = sample.evaluate(result.wi);
    //            print(eval);
    //        }
    //    }
}

}  // namespace scene::material::glass::testing

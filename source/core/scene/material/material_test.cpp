#include "material_test.hpp"
#include "base/math/print.inl"
#include "base/math/vector3.inl"
#include "bxdf.hpp"

#include <iostream>

namespace scene::material::testing {

static void print_vector(float2 v) {
    std::cout << v << " |" << length(v) << "|" << std::endl;
}

void print_vector(float3_p v) {
    std::cout << v << " |" << length(v) << "|" << std::endl;
}

bool check(float3_p result, float3_p h, float n_dot_wi, float n_dot_wo, float wo_dot_h, float pdf,
           const Layer& layer) {
    if (!std::isfinite(pdf) || !all_finite_and_positive(result)) {
        std::cout << "h: ";
        print_vector(h);
        std::cout << "n_dot_wi: " << n_dot_wi << std::endl;
        std::cout << "n_dot_wo: " << n_dot_wo << std::endl;
        std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
        std::cout << "pdf: " << pdf << std::endl;
        std::cout << "reflection " << result << std::endl;
        std::cout << "t ";
        print_vector(layer.t_);
        std::cout << "b ";
        print_vector(layer.b_);
        std::cout << "n ";
        print_vector(layer.n_);
        return false;
    }

    return true;
}

bool check(float3_p result, float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
           float pdf, const Layer& layer) {
    if (!std::isfinite(pdf) || !all_finite_and_positive(result)) {
        std::cout << "n_dot_wi: " << n_dot_wi << std::endl;
        std::cout << "n_dot_wo: " << n_dot_wo << std::endl;
        std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
        std::cout << "n_dot_h: " << n_dot_h << std::endl;
        std::cout << "pdf: " << pdf << std::endl;
        std::cout << "reflection " << result << std::endl;
        std::cout << "t ";
        print_vector(layer.t_);
        std::cout << "b ";
        print_vector(layer.b_);
        std::cout << "n ";
        print_vector(layer.n_);
        return false;
    }

    return true;
}

bool check(float3_p result, float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
           float pdf) {
    if (!std::isfinite(pdf) || !all_finite_and_positive(result)) {
        std::cout << "n_dot_wi: " << n_dot_wi << std::endl;
        std::cout << "n_dot_wo: " << n_dot_wo << std::endl;
        std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
        std::cout << "n_dot_h: " << n_dot_h << std::endl;
        std::cout << "pdf: " << pdf << std::endl;
        std::cout << "reflection " << result << std::endl;
        return false;
    }

    return true;
}

bool check(const bxdf::Sample& result, float3_p wo, const Layer& layer) {
    if (!std::isfinite(result.pdf) || !all_finite(result.wi) || !all_finite(result.reflection)) {
        std::cout << "wi: ";
        print_vector(result.wi);
        std::cout << "wo: ";
        print_vector(wo);
        std::cout << "h: ";
        print_vector(result.h);
        //	std::cout << "n_dot_wi: " << n_dot_wi << std::endl;
        //	std::cout << "n_dot_wo: " << n_dot_wo << std::endl;
        //	std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
        std::cout << "pdf: " << result.pdf << std::endl;
        std::cout << "reflection " << result.reflection << std::endl;
        std::cout << "t ";
        print_vector(layer.t_);
        std::cout << "b ";
        print_vector(layer.b_);
        std::cout << "n ";
        print_vector(layer.n_);
        return false;
    }

    return true;
}

bool check(const bxdf::Sample& result, float3_p wo, float n_dot_wi, float n_dot_wo, float wo_dot_h,
           const Layer& layer) {
    if (!std::isfinite(result.pdf) || !all_finite(result.wi) || !all_finite(result.reflection)) {
        std::cout << "wi: ";
        print_vector(result.wi);
        std::cout << "wo: ";
        print_vector(wo);
        std::cout << "h: ";
        print_vector(result.h);
        std::cout << "n_dot_wi: " << n_dot_wi << std::endl;
        std::cout << "n_dot_wo: " << n_dot_wo << std::endl;
        std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
        std::cout << "pdf: " << result.pdf << std::endl;
        std::cout << "reflection " << result.reflection << std::endl;
        std::cout << "t ";
        print_vector(layer.t_);
        std::cout << "b ";
        print_vector(layer.b_);
        std::cout << "n ";
        print_vector(layer.n_);
        return false;
    }

    return true;
}

bool check_normal_map(float3_p n, float2 tangent_space_n, float2 uv) {
    if (!all_finite(n)) {
        std::cout << "n: ";
        print_vector(n);
        std::cout << "ts_n: ";
        print_vector(tangent_space_n);
        std::cout << "uv: " << uv << std::endl;
        return false;
    }

    return true;
}

}  // namespace scene::material::testing

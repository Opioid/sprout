#pragma once

#include "postprocessor.hpp"

namespace rendering::postprocessor {

class Glare2 : public Postprocessor {
  public:
    enum class Adaption { Scotopic, Mesopic, Photopic };

    Glare2(Adaption adaption, float threshold, float intensity);
    ~Glare2();

    virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) override final;

    virtual size_t num_bytes() const override final;

  private:
    virtual void pre_apply(const image::Float4& source, image::Float4& destination,
                           thread::Pool& pool) override final;

    virtual void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end,
                       const image::Float4& source, image::Float4& destination) override final;

    Adaption adaption_;
    float    threshold_;
    float    intensity_;

    int2 kernel_dimensions_;

    float2* kernel_dft_r_;
    float2* kernel_dft_g_;
    float2* kernel_dft_b_;

    float* high_pass_r_;
    float* high_pass_g_;
    float* high_pass_b_;

    float2* high_pass_dft_r_;
    float2* high_pass_dft_g_;
    float2* high_pass_dft_b_;

    float2* temp_;
};

}  // namespace rendering::postprocessor

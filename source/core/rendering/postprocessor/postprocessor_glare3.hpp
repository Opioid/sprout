#pragma once

#include "postprocessor.hpp"

namespace rendering::postprocessor {

class Glare3 : public Postprocessor {
  public:
    enum class Adaption { Scotopic, Mesopic, Photopic };

    Glare3(Adaption adaption, float threshold, float intensity);
    ~Glare3();

    virtual void init(scene::camera::Camera const& camera, thread::Pool& pool) override final;

    virtual size_t num_bytes() const override final;

  private:
    virtual void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end,
                       const image::Float4& source, image::Float4& destination) override final;

    Adaption adaption_;
    float    threshold_;
    float    intensity_;

    //	image::Float3 high_pass_;
    int2    dimensions_;
    float3* high_pass_;

    int2    kernel_dimensions_;
    float3* kernel_;

    struct K {
        int32_t o;
        float   w;
    };

    int32_t gauss_width_;
    K*      gauss_kernel_;
};

}  // namespace rendering::postprocessor

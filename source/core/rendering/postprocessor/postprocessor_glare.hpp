#pragma once

#include "postprocessor.hpp"

namespace rendering::postprocessor {

class Glare : public Postprocessor {
  public:
    enum class Adaption { Scotopic, Mesopic, Photopic };

    Glare(Adaption adaption, float threshold, float intensity);
    ~Glare();

    virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) override final;

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
};

}  // namespace rendering::postprocessor

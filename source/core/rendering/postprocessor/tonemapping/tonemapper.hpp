#pragma once

#include "rendering/postprocessor/postprocessor.hpp"

namespace rendering {
namespace postprocessor {
namespace tonemapping {

class Tonemapper : public Postprocessor {
  public:
    virtual ~Tonemapper();

    virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) override final;

    virtual size_t num_bytes() const override final;

  protected:
    static float normalization_factor(float hdr_max, float tonemapped_max);
};

}  // namespace tonemapping
}  // namespace postprocessor
}  // namespace rendering

#ifndef SU_CORE_RENDERING_POSTPROCESSOR_GLARE_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_GLARE_HPP

#include "postprocessor.hpp"

namespace rendering::postprocessor {

class Glare : public Postprocessor {
  public:
    enum class Adaption { Scotopic, Mesopic, Photopic };

    Glare(Adaption adaption, float threshold, float intensity, float radius);

    ~Glare() override final;

    void init(scene::camera::Camera const& camera, thread::Pool& pool) override final;

    size_t num_bytes() const override final;

  private:
    void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end, image::Float4 const& source,
               image::Float4& destination) override final;

    Adaption adaption_;

    float threshold_;
    float intensity_;
    float radius_;

    int2 dimensions_;

    float3* high_pass_;

    float3* kernel_;
};

}  // namespace rendering::postprocessor

#endif

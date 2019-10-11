#ifndef SU_CORE_RENDERING_POSTPROCESSOR_GLARE_STOCHASTIC_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_GLARE_STOCHASTIC_HPP

#include "postprocessor.hpp"

namespace rendering::postprocessor {

class Glare_stochastic : public Postprocessor {
  public:
    enum class Adaption { Scotopic, Mesopic, Photopic };

    Glare_stochastic(Adaption adaption, float threshold, float intensity);

    ~Glare_stochastic() override final;

    void init(scene::camera::Camera const& camera, thread::Pool& pool) override final;

    size_t num_bytes() const override final;

  private:
    void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end, image::Float4 const& source,
               image::Float4& destination) override final;

    Adaption adaption_;

    float threshold_;
    float intensity_;

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

    K* gauss_kernel_;
};

}  // namespace rendering::postprocessor

#endif

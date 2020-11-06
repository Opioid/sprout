#ifndef SU_CORE_RENDERING_POSTPROCESSOR_GLARE_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_GLARE_HPP

#include "base/math/vector2.hpp"
#include "base/memory/array.hpp"
#include "postprocessor.hpp"

namespace rendering::postprocessor {

class Glare final : public Postprocessor {
  public:
    enum class Adaption { Scotopic, Mesopic, Photopic };

    Glare(Adaption adaption, float threshold, float intensity);

    ~Glare() final;

    void init(Camera const& camera, Threads& threads) final;

  private:
    void pre_apply(image::Float4 const& source, image::Float4& destination, Threads& threads) final;

    void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end, image::Float4 const& source,
               image::Float4& destination) final;

    void post_pass(uint32_t pass) final;

    Adaption adaption_;

    float threshold_;
    float intensity_;

    int2 dimensions_;

    memory::Concurrent_array<int2> high_;

    float3* kernel_;
};

}  // namespace rendering::postprocessor

#endif

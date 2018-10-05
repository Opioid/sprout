#ifndef SU_CORE_RENDERING_POSTPROCESSOR_BLOOM_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_BLOOM_HPP

#include <vector>
#include "image/typed_image.hpp"
#include "postprocessor.hpp"

namespace rendering::postprocessor {

class Bloom : public Postprocessor {
  public:
    Bloom(float angle, float alpha, float threshold, float intensity);

    virtual void init(scene::camera::Camera const& camera, thread::Pool& pool) override final;

    virtual size_t num_bytes() const override final;

  private:
    virtual void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end,
                       const image::Float4& source, image::Float4& destination) override final;

    float angle_;
    float alpha_;
    float threshold_;
    float intensity_;

    image::Float4 scratch_;

    struct K {
        int32_t o;
        float   w;
    };

    std::vector<K> kernel_;
};

}  // namespace rendering::postprocessor

#endif

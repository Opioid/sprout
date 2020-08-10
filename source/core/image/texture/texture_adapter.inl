#ifndef SU_CORE_IMAGE_TEXTURE_ADAPTER_INL
#define SU_CORE_IMAGE_TEXTURE_ADAPTER_INL

#include "base/math/vector4.inl"
#include "scene/scene.inl"
#include "scene/scene_worker.inl"
#include "texture_adapter.hpp"
#include "texture_sampler.hpp"

namespace image::texture {

inline Adapter::Adapter() : texture_(0xFFFFFFFF) {}

inline Adapter::Adapter(uint32_t texture) : texture_(texture), scale_(1.f) {}

inline Adapter::Adapter(uint32_t texture, float scale) : texture_(texture), scale_(scale) {}

inline Adapter::~Adapter() {}

inline bool Adapter::operator==(Adapter const& other) const {
    return texture_ == other.texture_ && scale_ == other.scale_;
}

inline bool Adapter::is_valid() const {
    return texture_ != 0xFFFFFFFF;
}

}  // namespace image::texture

#endif

#ifndef SU_CORE_IMAGE_PROCEDURAL_RENDERER_HPP
#define SU_CORE_IMAGE_PROCEDURAL_RENDERER_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector4.hpp"
#include "image/typed_image_fwd.hpp"

namespace image::procedural {

template <typename T>
class Typed_renderer {
  public:
    Typed_renderer(int2 dimensions, int32_t sqrt_num_samples = 1);

    ~Typed_renderer();

    void set_brush(T color);

    void clear();

    void draw_circle(float2 pos, float radius);

    void resolve(Typed_image<T>& target) const;

  private:
    void set_sample(int32_t x, int32_t y, T color);

    void set_row(int32_t start_x, int32_t end_x, int32_t y, T color);

    int32_t sqrt_num_samples_;

    int2 dimensions_;

    float2 dimensions_f_;

    T* samples_;

    T brush_;
};

class Renderer {
  public:
    Renderer(int2 dimensions, int32_t sqrt_num_samples = 1);

    ~Renderer();

    void set_brush(float3 const& color);
    void set_brush(float4 const& color);

    void clear();

    void draw_circle(float2 pos, float radius);

    void draw_circle(float2 pos, float radius, int32_t border);

    void draw_bounding_square(float2 pos, float radius);

    void draw_disk(float2 pos, float3 const& normal, float radius);

    void draw_disk(float2 pos, float3 const& normal, float radius, int32_t border);

    void draw_n_gon(float2 pos, float angle, float radius, uint32_t num_vertices);

    void resolve_sRGB(Byte3& image) const;

    void resolve(Byte3& image) const;

    void resolve(Byte1& image) const;

  private:
    void set_sample(int32_t x, int32_t y, float4 const& color);

    void set_row(int32_t start_x, int32_t end_x, int32_t y, float4 const& color);

    static bool intersect_disk(float2 pos, float3 const& normal, float radius,
                               float2 sample) noexcept;

    static bool intersect_n_gon(float2 pos, float angle, float radius, uint32_t num_vertices,
                                float2 sample) noexcept;

    int32_t const sqrt_num_samples_;

    int2 dimensions_;

    float2 dimensions_f_;

    float4* samples_;

    float4 brush_;
};

}  // namespace image::procedural

#endif

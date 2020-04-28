#ifndef SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_HPP
#define SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_HPP

#include <cstdint>

namespace scene::material {

class Sample_cache {
  public:
    Sample_cache();

    ~Sample_cache();

    template <typename T>
    T& get();

    static uint32_t constexpr Max_sample_size = 512;

  private:
    char buffer_[Max_sample_size];
};

}  // namespace scene::material

#endif

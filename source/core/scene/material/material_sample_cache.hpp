#ifndef SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_HPP
#define SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_HPP

#include <cstdint>

namespace scene::material {

class Sample_cache {
  public:
    Sample_cache(uint32_t max_sample_size);

    ~Sample_cache();

    template <typename T>
    T& get();

  private:
    uint32_t max_sample_size_;

    char* buffer_;
};

}  // namespace scene::material

#endif

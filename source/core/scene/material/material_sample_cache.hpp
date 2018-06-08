#ifndef SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_HPP
#define SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_HPP

#include <cstddef>
#include <cstdint>

namespace scene::material {

class Sample_cache {
 public:
  ~Sample_cache();

  void init(uint32_t max_sample_size);

  template <typename T>
  T& get();

  size_t num_bytes() const;

 private:
  uint32_t buffer_size_ = 0;
  char* buffer_ = nullptr;
};

}  // namespace scene::material

#endif

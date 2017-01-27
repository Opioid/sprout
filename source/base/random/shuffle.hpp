#pragma once

namespace rnd {

class Generator;

template<typename T>
void shuffle(T* data, uint32_t count, Generator& rng);

template<typename T>
void shuffle2(T* data, uint32_t count, Generator& rng);

}

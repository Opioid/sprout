#pragma once

namespace rnd {

class Generator;

template<typename T>
void shuffle(T* data, uint32_t count, Generator& rng);

}

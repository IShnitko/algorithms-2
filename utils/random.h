#ifndef RANDOM_H
#define RANDOM_H

#include <cstdint>
#include <cstdlib>
#include <ctime>

inline void init_random() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

inline uint32_t rand_range(uint32_t min, uint32_t max) {
    return min + (std::rand() % (max - min + 1));
}

#endif // RANDOM_H
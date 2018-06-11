#ifndef SU_BASE_DEBUG_ASSERT_HPP
#define SU_BASE_DEBUG_ASSERT_HPP

#ifndef SU_DEBUG
#define SOFT_ASSERT(EXPRESSION) ((void)0)
#else
#include <iostream>
inline void print_location(char const* file, int line) {
    std::cout << file << "(" << line << ")" << std::endl;
}
#define SOFT_ASSERT(EXPRESSION) ((EXPRESSION) ? (void)0 : print_location(__FILE__, __LINE__))
#endif

#endif

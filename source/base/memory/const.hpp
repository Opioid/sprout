#ifndef SU_BASE_MEMORY_CONST_HPP
#define SU_BASE_MEMORY_CONST_HPP

// The purpose of the following global constants is to prevent redundant
// reloading of the constants when they are referenced by more than one
// separate inline math routine called within the same function.  Declaring
// a constant locally within a routine is sufficient to prevent redundant
// reloads of that constant when that single routine is called multiple
// times in a function, but if the constant is used (and declared) in a
// separate math routine it would be reloaded.

#ifndef SU_GLOBALCONST
#ifdef __GNUG__
#define SU_GLOBALCONST(X) extern const X __attribute__((weak))
#elif defined(_MSC_VER)
#define SU_GLOBALCONST(X) extern const __declspec(selectany) X
#endif
#endif

#endif

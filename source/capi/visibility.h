#ifndef SU_CAPI_VISIBILITY_H
#define SU_CAPI_VISIBILITY_H

#if defined _WIN32
#ifdef SU_LIBRARY_EXPORTS
#define SU_LIBRARY_API __declspec(dllexport)
#else
#define SU_LIBRARY_API __declspec(dllimport)
#endif
#else
#define SU_LIBRARY_API __attribute__ ((visibility ("default")))
#endif

#endif

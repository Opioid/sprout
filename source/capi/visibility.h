#ifndef SU_CAPI_VISIBILITY_H
#define SU_CAPI_VISIBILITY_H

#if defined _WIN32
  #define DLL_PUBLIC __declspec(dllexport)
  #define DLL_LOCAL
#else
  #define DLL_PUBLIC __attribute__ ((visibility ("default")))
  #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#endif

#endif

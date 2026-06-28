#ifndef __khrplatform_h_
#define __khrplatform_h_
typedef          float         khronos_float_t;
typedef signed   char          khronos_int8_t;
typedef unsigned char          khronos_uint8_t;
typedef signed   short int     khronos_int16_t;
typedef unsigned short int     khronos_uint16_t;
typedef signed   int           khronos_int32_t;
typedef unsigned int           khronos_uint32_t;
typedef signed   long long int khronos_int64_t;
typedef unsigned long long int khronos_uint64_t;
typedef          long int       khronos_intptr_t;
typedef unsigned long int       khronos_uintptr_t;
typedef          long int       khronos_ssize_t;
typedef unsigned long int       khronos_usize_t;
#ifndef KHRONOS_SUPPORT_INT64
# define KHRONOS_SUPPORT_INT64   1
#endif
#ifndef KHRONOS_SUPPORT_FLOAT
# define KHRONOS_SUPPORT_FLOAT   1
#endif
#if defined(__SCITECH_SNAP__) && !defined(KHRONOS_STATIC)
#   define KHRONOS_STATIC
#endif
#ifdef KHRONOS_STATIC
#   define KHRONOS_APICALL
#elif defined(_WIN32)
#   define KHRONOS_APICALL __declspec(dllimport)
#elif defined (__SYMBIAN32__)
#   define KHRONOS_APICALL IMPORT_C
#elif defined(__ANDROID__)
#   define KHRONOS_APICALL __attribute__((visibility("default")))
#else
#   define KHRONOS_APICALL
#endif
#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(__SCITECH_SNAP__)
#   define KHRONOS_APIENTRY __stdcall
#else
#   define KHRONOS_APIENTRY
#endif
#ifndef KHRONOS_APIATTRIBUTES
#define KHRONOS_APIATTRIBUTES
#endif
#define KHRONOS_TRUE  1
#define KHRONOS_FALSE 0
#endif /* __khrplatform_h_ */

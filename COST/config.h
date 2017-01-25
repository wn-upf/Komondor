#define HAVE_INT8_T 1
#define HAVE_INT16_T 1
#define HAVE_INT32_T 1
#define HAVE_INT64_T 1

#define HAVE_UINT8_T 1
#define HAVE_UINT16_T 1
#define HAVE_UINT32_T 1
#define HAVE_UINT64_T 1

#if 1
#include <inttypes.h>
#endif

#if !HAVE_INT8_T
typedef signed char int8_t;
#endif
#if !HAVE_INT16_T
typedef signed short int16_t;
#endif
#if !HAVE_INT32_T
typedef signed long int32_t;
#endif
#if !HAVE_INT64_T
typedef signed long long int64_t;
#endif


#if !HAVE_UINT8_T
typedef unsigned char uint8_t;
#endif
#if !HAVE_UINT16_T
typedef unsigned short uint16_t;
#endif
#if !HAVE_UINT32_T
typedef unsigned long uint32_t;
#endif
#if !HAVE_UINT64_T
typedef unsigned long long uint64_t;
#endif



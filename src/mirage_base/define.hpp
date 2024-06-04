#ifndef MIRAGE_BASE_DEFINE
#define MIRAGE_BASE_DEFINE

#if defined(DEBUG)
#include <cassert>
#endif

#if defined(MIRAGE_BUILD_SHARED) && defined(_WIN32)
#if defined(MIRAGE_BUILD)
#define MIRAGE_API __declspec(dllexport)
#else
#define MIRAGE_API __declspec(dllimport)
#endif
#else
#define MIRAGE_API
#endif

#if defined(DEBUG)
#define MIRAGE_DCHECK(condition) assert(condition)
#else
#define MIRAGE_DCHECK(condition) ((void)0)
#endif

#endif  // MIRAGE_BASE_DEFINE

#ifndef __GAME_DEFINE__
#define __GAME_DEFINE__
#include <cstdint>
#pragma warning(disable:4996) // disable C Run-Time library deprecation warnings

/** DEFINES AND TYPEDEFS **/
/* Put INTERNAL in front of all functions, and that prevents them from ever going into the linking table.
This causes the compiler to treat them as intra-unit linkage and it doesn't ever have to even do the work
 of moving to the link phase. */
#define INTERNAL static             // static functions are internal to the translation unit
#define LOCAL_PERSIST static        // local static variables are only accessible within the scope of its declaration
#define GLOBAL_VAR static           // global static variables are global to the translation unit

typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef uint64_t    uint64;

typedef int8_t      int8;
typedef int16_t     int16;
typedef int32_t     int32;
typedef int64_t     int64;

typedef float       real32;
typedef double      real64;

#define INDEX_NONE -1
#define ASCII_SPACE 32
#define ASCII_TILDE 126

/** HELPER FUNCTIONS **/
#if SLOW_BUILD
#define ASSERT(predicate) if(!(predicate)) {*(int*)0 = 0;}
#else
#define ASSERT(predicate)
#endif

#define TYPEHASH(T) typeid(T).hash_code()
#define array_count(arr) (sizeof(arr) / (sizeof(arr[0])))
#define loop(x) for(int i=0;i<x;++i)

/** GAME SPECIFIC **/
#define WIDTH 1792
#define HEIGHT 1008

#endif
#ifndef __GAME_DEFINE__
#define __GAME_DEFINE__

#include <cstdint>

/** Defines and typedefs for game */

#define INTERNAL static				// static functions are internal to the translation unit
#define LOCAL_PERSIST static		// local static variables are only accessible within the scope of its declaration
#define GLOBAL_VAR static			// global static variables are global to the translation unit

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef float float32;
typedef double float64;
typedef size_t mi;			// memory index

const float TO_RADIANS = 3.14159265f / 180.f; // in_degrees * to_radians = in_radians
#define INDEX_NONE -1;


#endif
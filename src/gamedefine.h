#ifndef __GAME_DEFINE__
#define __GAME_DEFINE__
#include <cstdint>

/** DEFINES AND TYPEDEFS **/
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

typedef size_t mi;					// memory index

const float TO_RADIANS = 0.0174532925f; // in_degrees * TO_RADIANS = in_radians
const float TO_DEGREES = 57.2958f;		// in_radians * TO_DEGREES = in_degrees

#define INDEX_NONE -1;


/** GAME SPECIFIC **/
const int32 WIDTH = 1280, HEIGHT = 720;


#endif
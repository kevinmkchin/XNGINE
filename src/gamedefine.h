#ifndef __GAME_DEFINE__
#define __GAME_DEFINE__

#include <cstdint>
#pragma warning(disable:4996) // disable C Run-Time library deprecation warnings

////////////////////////////////////////////////////////////////////////////////////
/** DEFINES AND TYPEDEFS **/
////////////////////////////////////////////////////////////////////////////////////

/* Put INTERNAL in front of all functions, and that prevents them from ever going into the linking table.
This causes the compiler to treat them as intra-unit linkage and it doesn't ever have to even do the work
 of moving to the link phase. */

#define INTERNAL        static  // functions or global variables marked static are internal to the translation unit
#define local_persist   static  // local static variables are only accessible within the scope of its declaration

typedef uint8_t       u8;
typedef uint16_t      u16;
typedef uint32_t      u32;
typedef uint64_t      u64;
typedef int8_t        i8;
typedef int16_t       i16;
typedef int32_t       i32;
typedef int64_t       i64;
typedef uint_fast8_t  u8f;
typedef uint_fast16_t u16f;
typedef uint_fast32_t u32f;
typedef uint_fast64_t u64f;
typedef int_fast8_t   i8f;
typedef int_fast16_t  i16f;
typedef int_fast32_t  i32f;
typedef int_fast64_t  i64f;
typedef i32           bool32;

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

#define kilobytes(value) ((value)*1024)
#define megabytes(value) (kilobytes((unsigned long long int)value)*1024)
#define gigabytes(value) (megabytes((unsigned long long int)value)*1024)

/** Singletons
    TODO: don't "new" in get_instance - have a separate function for allocating singleton
    TODO: maybe stop using these #defines at some point
    A.h:
    struct A
    {
        void foo();
        int bar = 1;
        SINGLETON(A); // at the end of class/struct declaration
    };

    A.cpp:
    ...
    SINGLETON_INIT(window_manager) // can go anywhere in source file
    ...
*/
#define SINGLETON(classname) \
    public: \
        static classname* get_instance() \
        { \
            if(singleton_ == nullptr) \
            { \
                singleton_ = new classname(); \
            } \
            return singleton_; \
        } \
        classname(classname &other) = delete; \
        void operator=(const classname &) = delete; \
    private: \
        classname() = default; \
        static classname* singleton_;

#define SINGLETON_INIT(classname) \
    classname* classname::singleton_ = nullptr;


////////////////////////////////////////////////////////////////////////////////////
/** GAME SPECIFIC **/
////////////////////////////////////////////////////////////////////////////////////

#define WIDTH 1600
#define HEIGHT 900

//enum gameobject_flags
//{
//    UPDATEACTIVE = 1 << 0,
//    COLLIDABLE   = 1 << 1,
//};

#endif //__GAME_DEFINE__
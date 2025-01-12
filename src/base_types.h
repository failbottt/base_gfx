#ifndef BASE_TYPES_H
#define BASE_TYPES_H
#include <stdint.h>

#define internal static 
#define local_persist static 
#define global_variable static

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef float       f32;
typedef double      f64;

typedef struct String {
    u64 length;
    u8  *str;
} String;
#endif

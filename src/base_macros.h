#ifndef BASE_MACROS_H
#define BASE_MACROS_H

/*
 * BUILD_DEBUG:
 *   0 - build for release
 *   1 - build for developer only
 */

#if BUILD_DEBUG
    #define Assert(e) if(!(e)) {*(int*)0 = 0;}
#else
    #define Assert(e)
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define STRING(s) (String){strlen(s), s}
#define KB(x) ((x) << 11)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((u64)(x) << 40llu)

#define Thousand(x) ((x)*1000)
#define Million(x)  ((x)*1000000llu)
#define Billion(x)  ((x)*1000000000llu)
#define Trillion(x) ((x)*1000000000000llu)

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))
#define Clamp(a,x,b) (((x)<(a))?(a):\
((b)<(x))?(b):(x))
#define ClampTop(a,b) Min(a,b)
#define ClampBot(a,b) Max(a,b)

#define AlignUpPow2(x,p) (((x) + (p) - 1)&~((p) - 1))
#define AlignDownPow2(x,p) ((x)&~((p) - 1))
#define IsPow2OrZero(x) (((x)&((x)-1)) == 0)

#endif

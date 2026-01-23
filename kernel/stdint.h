/* include/stdint.h - 裸机开发专用 */
#ifndef _STDINT_H
#define _STDINT_H

/* 精确宽度整数类型 */
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

/* 处理器字长的类型 */
typedef int32_t            intptr_t;
typedef uint32_t           uintptr_t;

/* 最大宽度类型 */
typedef int64_t            intmax_t;
typedef uint64_t          uintmax_t;

/* 指针差值类型 */
typedef int32_t            ptrdiff_t;

/* 大小类型 */
typedef uint32_t           size_t;

/* 空指针 */
#define NULL ((void*)0)

#endif /* _STDINT_H */
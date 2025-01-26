#pragma once

#define ALIGN(x) __attribute__((aligned(x)))

#if defined(__builtin_expect)
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif

enum {
	ERR_OK = 0,
};

typedef int err;


#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#define FRACTION 16384      // 1 << 14

#define CONVERT_TO_FP(n) (n) * (FRACTION)
#define CONVERT_TO_INT_ZERO(x) (x) / (FRACTION)
#define CONVERT_TO_INT_NEAR(x) (x >= 0) ? ((x) + (FRACTION) / 2) / (FRACTION) : ((x) - (FRACTION) / 2) / (FRACTION)
#define FP_ADD(x, y) (x) + (y)
#define FP_SUB(x, y) (x) - (y)
#define FP_ADD_INT(x, n) (x) + (n) * (FRACTION)
#define FP_SUB_INT(x, n) (x) - (n) * (FRACTION)
#define FP_MUL(x, y) ((int64_t)(x)) * (y) / (FRACTION)
#define FP_MUL_INT(x, n) (x) * (n)
#define FP_DIV(x, y) ((int64_t)(x)) * (FRACTION) / (y)
#define FP_DIV_INT(x, n) (x) / (n)

#endif /* threads/fixed-point.h */
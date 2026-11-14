#pragma once
// The original helper uses ordered scalar SSE, including unordered comparisons.
#if defined(__FAST_MATH__) || defined(__FMA__)
#error Fort jump math requires the validated scalar SSE compilation settings.
#endif
#include <xmmintrin.h>
#include <cmath>
#include <cfloat>

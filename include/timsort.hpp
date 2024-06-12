#pragma once
#ifndef GFX_TIMSORT_WRAPPER
#define GFX_TIMSORT_WRAPPER

// clang-format off
// #if __has_include(<ranges> )
//   #include "gfx/timsort3.hpp"
// #else
//   #include "gfx/timsort2.hpp"
// #endif
// Apple Clang 14 arm failed to check ranges
#include "gfx/timsort2.hpp"
// clang-format on

#endif   // GFX_TIMSORT_WRAPPER

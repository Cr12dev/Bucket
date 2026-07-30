#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #define BUCKET_PLATFORM_WINDOWS
#elif defined(__APPLE__)
  #define BUCKET_PLATFORM_MACOS
#elif defined(__linux__) || defined(__unix__)
  #define BUCKET_PLATFORM_LINUX
#else
  #error "Unsupported platform"
#endif

#if defined(BUCKET_PLATFORM_WINDOWS)
  #define BUCKET_BREAK() __debugbreak()
#elif defined(BUCKET_PLATFORM_MACOS) || defined(BUCKET_PLATFORM_LINUX)
  #include <signal.h>
  #define BUCKET_BREAK() raise(SIGTRAP)
#endif

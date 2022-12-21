#pragma once

//
// Compiler type definitions.
//

#define M_COMPILER_MSVC	            1
#define M_COMPILER_GCC              2
#define M_COMPILER_CLANG            3

//
// Architecture definitions.
//

#define M_COMPILER_ARCH_32          0
#define M_COMPILER_ARCH_64          1

#define M_IS_ARCH_64                (M_COMPILER_ARCH == M_COMPILER_ARCH_64)
#define M_IS_ARCH_32                (!M_IS_ARCH_64)

//
// Target OS definitions.
//

#define M_TARGET_OS_WINDOWS         1
#define M_TARGET_OS_LINUX           2


//
// Test compiler type and architecture.
//

#if defined(_MSC_VER)
#define M_COMPILER_TYPE             M_COMPILER_MSVC
#if (defined(_M_AMD64) || defined(_M_X64) || defined(_M_IA64)) /* for MSVC */
#define M_COMPILER_ARCH             M_COMPILER_ARCH_64
#else
#define M_COMPILER_ARCH             M_COMPILER_ARCH_32
#endif
#elif defined(__GNUC__)
#define M_COMPILER_TYPE             M_COMPILER_GCC
#if (defined(__x86_64__) || defined(__ia64) || defined(__aarch64__) || defined(__powerpc64__)) /* for GCC */
#define M_COMPILER_ARCH             M_COMPILER_ARCH_64
#else
#define M_COMPILER_ARCH             M_COMPILER_ARCH_32
#endif
#else
#error compiler not supported
#endif

//
// Test target OS.
//

#ifdef M_TARGET_OS
#if M_TARGET_OS == M_TARGET_OS_WINDOWS
// Windows specific
#elif M_TARGET_OS == M_TARGET_OS_LINUX
// Linux specific
#else
#error Unknown target OS
#endif
#else
#error Please define corresponding macro M_TARGET_OS for target OS
#endif

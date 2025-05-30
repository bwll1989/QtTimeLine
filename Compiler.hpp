#pragma once

#if defined(__MINGW32__) || defined(__MINGW64__)
#define NODE_TIMELINE_COMPILER "MinGW"
#define NODE_TIMELINE_COMPILER_MINGW
#elif defined(__clang__)
#define NODE_TIMELINE_COMPILER "Clang"
#define NODE_TIMELINE_COMPILER_CLANG
#elif defined(_MSC_VER)
#define NODE_TIMELINE_COMPILER "Microsoft Visual C++"
#define NODE_TIMELINE_COMPILER_MICROSOFT
#elif defined(__GNUC__)
#define NODE_TIMELINE_COMPILER "GNU"
#define NODE_TIMELINE_COMPILER_GNU
#define NODE_TIMELINE_COMPILER_GNU_VERSION_MAJOR __GNUC__
#define NODE_TIMELINE_COMPILER_GNU_VERSION_MINOR __GNUC_MINOR__
#define NODE_TIMELINE_COMPILER_GNU_VERSION_PATCH __GNUC_PATCHLEVEL__
#elif defined(__BORLANDC__)
#define NODE_TIMELINE_COMPILER "Borland C++ Builder"
#define NODE_TIMELINE_COMPILER_BORLAND
#elif defined(__CODEGEARC__)
#define NODE_TIMELINE_COMPILER "CodeGear C++ Builder"
#define NODE_TIMELINE_COMPILER_CODEGEAR
#elif defined(__INTEL_COMPILER) || defined(__ICL)
#define NODE_TIMELINE_COMPILER "Intel C++"
#define NODE_TIMELINE_COMPILER_INTEL
#elif defined(__xlC__) || defined(__IBMCPP__)
#define NODE_TIMELINE_COMPILER "IBM XL C++"
#define NODE_TIMELINE_COMPILER_IBM
#elif defined(__HP_aCC)
#define NODE_TIMELINE_COMPILER "HP aC++"
#define NODE_TIMELINE_COMPILER_HP
#elif defined(__WATCOMC__)
#define NODE_TIMELINE_COMPILER "Watcom C++"
#define NODE_TIMELINE_COMPILER_WATCOM
#endif

#ifndef NODE_TIMELINE_COMPILER
#error "Current compiler is not supported."
#endif

#pragma once

#include "Compiler.hpp"
#include "OperatingSystem.hpp"

#ifdef NODE_TIMELINE_PLATFORM_WINDOWS
#define NODE_TIMELINE_EXPORT __declspec(dllexport)
#define NODE_TIMELINE_IMPORT __declspec(dllimport)
#define NODE_TIMELINE_LOCAL
#elif NODE_TIMELINE_COMPILER_GNU_VERSION_MAJOR >= 4 || defined(NODE_TIMELINE_COMPILER_CLANG)
#define NODE_TIMELINE_EXPORT __attribute__((visibility("default")))
#define NODE_TIMELINE_IMPORT __attribute__((visibility("default")))
#define NODE_TIMELINE_LOCAL __attribute__((visibility("hidden")))
#else
#define NODE_TIMELINE_EXPORT
#define NODE_TIMELINE_IMPORT
#define NODE_TIMELINE_LOCAL
#endif

#ifdef __cplusplus
#define NODE_TIMELINE_DEMANGLED extern "C"
#else
#define NODE_TIMELINE_DEMANGLED
#endif

#if defined(NODE_TIMELINE_SHARED) && !defined(NODE_TIMELINE_STATIC)
#ifdef NODE_TIMELINE_EXPORTS
#define NODE_TIMELINE_PUBLIC NODE_TIMELINE_EXPORT
#else
#define NODE_TIMELINE_PUBLIC NODE_TIMELINE_IMPORT
#endif
#define NODE_TIMELINE_PRIVATE NODE_TIMELINE_LOCAL
#elif !defined(NODE_TIMELINE_SHARED) && defined(NODE_TIMELINE_STATIC)
#define NODE_TIMELINE_PUBLIC
#define NODE_TIMELINE_PRIVATE
#elif defined(NODE_TIMELINE_SHARED) && defined(NODE_TIMELINE_STATIC)
#ifdef NODE_TIMELINE_EXPORTS
#error "Cannot build as shared and static simultaneously."
#else
#error "Cannot link against shared and static simultaneously."
#endif
#else
#ifdef NODE_TIMELINE_EXPORTS
#error "Choose whether to build as shared or static."
#else
#error "Choose whether to link against shared or static."
#endif
#endif

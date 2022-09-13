#ifndef XRMEMORY_PURE_H
#define XRMEMORY_PURE_H

#ifdef XRCORE_STATIC
#	define PURE_ALLOC
#else // XRCORE_STATIC
#	ifdef DEBUG
#		define PURE_ALLOC
#	endif // DEBUG
#endif // XRCORE_STATIC

#ifdef _M_X64
#undef PURE_ALLOC
#define PURE_ALLOC
#endif

#endif // XRMEMORY_PURE_H
#pragma once

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
	#define AGGE_INLINE __forceinline
#elif defined(__GNUC__) && (defined(__x86_64) || defined(__i386))
	#define AGGE_INLINE __attribute__((always_inline))
#else
	#define AGGE_INLINE inline
#endif

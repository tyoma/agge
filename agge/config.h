#pragma once

#if defined(_M_X64) || defined(__x86_64) || defined(_M_IX86) || defined(__i386)
	#define AGGE_INLINE __forceinline
#else
	#define AGGE_INLINE inline
#endif

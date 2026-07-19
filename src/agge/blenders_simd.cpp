#include <agge/config.h>

#if defined(AGGE_ARCH_INTEL)
	#include "blenders_intel.cpp"
#elif defined(AGGE_ARCH_ARM)
	#include "blenders_arm.cpp"
#else
#endif

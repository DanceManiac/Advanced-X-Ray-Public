#ifndef __XR_MATH_H__
#define __XR_MATH_H__

#include "cpuid.h"

namespace CPU {
	XRCORE_API extern u64				clk_per_second		;
	XRCORE_API extern u64				qpc_freq			;
	XRCORE_API extern u32				qpc_counter			;

	XRCORE_API extern	processor_info	Info				;
	XRCORE_API extern	u64				QPC	()	noexcept	;

	IC u64	GetCLK(void)
	{
		return __rdtsc();
	}

	IC void CachePtrToL1(void* p)
	{
		if (Info.hasFeature(CPUFeature::AMD_3DNow))
			_m_prefetchw(p);
	}
};

XRCORE_API void _initialize_cpu();
XRCORE_API void set_current_thread_name(const char* threadName);
XRCORE_API void set_thread_name(const char* threadName, std::thread& thread);

#endif //__XR_MATH_H__

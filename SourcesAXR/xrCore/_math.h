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

extern XRCORE_API	void	_initialize_cpu			();
extern XRCORE_API	void	_initialize_cpu_thread	();

// threading
typedef				void	thread_t				( void * );
extern XRCORE_API	void	thread_name				( const char* name);
extern XRCORE_API	void	thread_spawn			(
	thread_t*	entry,
	const char*	name,
	unsigned	stack,
	void*		arglist 
	);

#endif //__XR_MATH_H__

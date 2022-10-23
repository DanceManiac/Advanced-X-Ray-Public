#include "stdafx.h"
#pragma hdrstop

#include <process.h>

// mmsystem.h
#define MMNOSOUND
#define MMNOMIDI
#define MMNOAUX
#define MMNOMIXER
#define MMNOJOY
#include <mmsystem.h>

// Initialized on startup
XRCORE_API	Fmatrix			Fidentity;
XRCORE_API	Dmatrix			Didentity;
XRCORE_API	CRandom			Random;

typedef struct _PROCESSOR_POWER_INFORMATION
{
	DWORD Number;
	DWORD MaxMhz;
	DWORD CurrentMhz;
	DWORD MhzLimit;
	DWORD MaxIdleState;
	DWORD CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, * PPROCESSOR_POWER_INFORMATION;

namespace FPU
{
	void initialize()
	{
		::Random.seed(u32(CPU::GetCLK() % (1i64 << 32i64)));
	}
};

namespace CPU
{
	XRCORE_API u64 qpc_freq;
	XRCORE_API u32 qpc_counter = 0;
	XRCORE_API processor_info Info;

	XRCORE_API u64 clk_per_second = 0;
	XRCORE_API u64 QPC() noexcept
	{
		u64 _dest;
		QueryPerformanceCounter(reinterpret_cast<PLARGE_INTEGER>(&_dest));
		qpc_counter++;
		return _dest;
	}

	void Detect()
	{
		// Timers & frequency
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

		u64 start = GetCLK();
		while (GetCLK() - start < 1000);
		u64 end = GetCLK();

		clk_per_second = end - start;

		// Detect RDTSC Overhead
		u64 clk_overhead = 0;
		for (u32 i = 0; i < 256; i++)
		{
			start = GetCLK();
			clk_overhead += GetCLK() - start;
		}

		clk_overhead /= 256;
		clk_per_second -= clk_overhead;

		// Detect QPC
		LARGE_INTEGER Freq;
		QueryPerformanceFrequency(&Freq);
		qpc_freq = Freq.QuadPart;

		// Restore normal priority
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	}
};

bool g_initialize_cpu_called = false;

//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
void _initialize_cpu(void)
{
	shared_str vendor = "Unknown";

	if (CPU::Info.isAmd)
		vendor = "AMD";
	else if (CPU::Info.isIntel)
		vendor = "Intel";

	Msg("* Vendor CPU: %s", *vendor);
	Msg("* Detected CPU: %s", CPU::Info.modelName);

	string256 features;
	xr_strcpy(features, sizeof(features), "RDTSC");
	
	if (CPU::Info.hasFeature(CPUFeature::MMX))
		xr_strcat(features, ", MMX");

	if (CPU::Info.hasFeature(CPUFeature::AMD_3DNow))
		xr_strcat(features, ", 3DNow!");

	if (CPU::Info.hasFeature(CPUFeature::AMD_3DNowExt))
		xr_strcat(features, ", 3DNowExt!");

	if (CPU::Info.hasFeature(CPUFeature::SSE))
		xr_strcat(features, ", SSE");

	if (CPU::Info.hasFeature(CPUFeature::SSE2))
		xr_strcat(features, ", SSE2");

	if (CPU::Info.hasFeature(CPUFeature::SSE3))
		xr_strcat(features, ", SSE3");

	if (CPU::Info.hasFeature(CPUFeature::MWait))
		xr_strcat(features, ", MONITOR/MWAIT");

	if (CPU::Info.hasFeature(CPUFeature::SSSE3))
		xr_strcat(features, ", SSSE3");

	if (CPU::Info.hasFeature(CPUFeature::SSE41))
		xr_strcat(features, ", SSE4.1");

	if (CPU::Info.hasFeature(CPUFeature::SSE42))
		xr_strcat(features, ", SSE4.2");

	if (CPU::Info.hasFeature(CPUFeature::HT))
		xr_strcat(features, ", HTT");

	if (CPU::Info.hasFeature(CPUFeature::AVX))
		xr_strcat(features, ", AVX");
#ifdef __AVX__
	else Debug.do_exit(NULL, "X-Ray x64 using AVX anyway!");
#endif

	if (CPU::Info.hasFeature(CPUFeature::AVX2))
		xr_strcat(features, ", AVX2");

	if (CPU::Info.hasFeature(CPUFeature::SSE4a))
		xr_strcat(features, ", SSE4.a");

	if (CPU::Info.hasFeature(CPUFeature::MMXExt))
		xr_strcat(features, ", MMXExt");

	if (CPU::Info.hasFeature(CPUFeature::TM2))
		xr_strcat(features, ", TM2");

	if (CPU::Info.hasFeature(CPUFeature::AES))
		xr_strcat(features, ", AES");

	if (CPU::Info.hasFeature(CPUFeature::VMX))
		xr_strcat(features, ", VMX");

	if (CPU::Info.hasFeature(CPUFeature::EST))
		xr_strcat(features, ", EST");

	if (CPU::Info.hasFeature(CPUFeature::XFSR))
		xr_strcat(features, ", XFSR");

	Msg("* CPU features: %s", features);
	Msg("* CPU cores/threads: %d/%d \n", CPU::Info.n_cores, CPU::Info.n_threads);

	// Per second and QPC, lol 
	CPU::Detect();

	Fidentity.identity();	// Identity matrix
	Didentity.identity();	// Identity matrix
	pvInitializeStatics();	// Lookup table for compressed normals
	FPU::initialize();
	_initialize_cpu_thread();

	g_initialize_cpu_called = true;
}


// per-thread initialization
#include <xmmintrin.h>
constexpr int _MM_DENORMALS_ZERO = 0x0040;
constexpr int _MM_FLUSH_ZERO = 0x8000;

inline void _mm_set_flush_zero_mode(u32 mode)
{
	_mm_setcsr((_mm_getcsr() & ~_MM_FLUSH_ZERO) | (mode));
}

inline void _mm_set_denormals_zero_mode(u32 mode)
{
	_mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO) | (mode));
}

static	bool _denormals_are_zero_supported = true;

extern void debug_on_thread_spawn();

void _initialize_cpu_thread()
{
	debug_on_thread_spawn();

	_mm_set_flush_zero_mode(_MM_FLUSH_ZERO);
	if (_denormals_are_zero_supported)
	{
		__try
		{
			_mm_set_denormals_zero_mode(_MM_DENORMALS_ZERO);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			_denormals_are_zero_supported = false;
		}
	}
}

// threading API 
#pragma pack(push,8)
struct THREAD_NAME	{
	DWORD	dwType;
	LPCSTR	szName;
	DWORD	dwThreadID;
	DWORD	dwFlags;
};
void	thread_name	(const char* name)
{
	THREAD_NAME		tn;
	tn.dwType		= 0x1000;
	tn.szName		= name;
	tn.dwThreadID	= DWORD(-1);
	tn.dwFlags		= 0;
	__try
	{
		RaiseException(0x406D1388,0,sizeof(tn)/sizeof(DWORD),(ULONG_PTR*)&tn);
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
#pragma pack(pop)

struct	THREAD_STARTUP
{
	thread_t*	entry	;
	char*		name	;
	void*		args	;
};
void	__cdecl			thread_entry	(void*	_params )	{
	// initialize
	THREAD_STARTUP*		startup	= (THREAD_STARTUP*)_params	;
	thread_name			(startup->name);
	thread_t*			entry	= startup->entry;
	void*				arglist	= startup->args;
	xr_delete			(startup);
	_initialize_cpu_thread		();

	// call
	entry				(arglist);
}

void	thread_spawn	(thread_t*	entry, const char*	name, unsigned	stack, void* arglist )
{
	Debug._initialize	(false);

	THREAD_STARTUP*		startup	= xr_new<THREAD_STARTUP>	();
	startup->entry		= entry;
	startup->name		= (char*)name;
	startup->args		= arglist;
	_beginthread		(thread_entry,stack,startup);
}

void spline1	( float t, Fvector *p, Fvector *ret )
{
	float     t2  = t * t;
	float     t3  = t2 * t;
	float     m[4];

	ret->x=0.0f;
	ret->y=0.0f;
	ret->z=0.0f;
	m[0] = ( 0.5f * ( (-1.0f * t3) + ( 2.0f * t2) + (-1.0f * t) ) );
	m[1] = ( 0.5f * ( ( 3.0f * t3) + (-5.0f * t2) + ( 0.0f * t) + 2.0f ) );
	m[2] = ( 0.5f * ( (-3.0f * t3) + ( 4.0f * t2) + ( 1.0f * t) ) );
	m[3] = ( 0.5f * ( ( 1.0f * t3) + (-1.0f * t2) + ( 0.0f * t) ) );

	for( int i=0; i<4; i++ )
	{
		ret->x += p[i].x * m[i];
		ret->y += p[i].y * m[i];
		ret->z += p[i].z * m[i];
	}
}

void spline2( float t, Fvector *p, Fvector *ret )
{
	float	s= 1.0f - t;
	float   t2 = t * t;
	float   t3 = t2 * t;
	float   m[4];

	m[0] = s*s*s;
	m[1] = 3.0f*t3 - 6.0f*t2 + 4.0f;
	m[2] = -3.0f*t3 + 3.0f*t2 + 3.0f*t +1;
	m[3] = t3;

	ret->x = (p[0].x*m[0]+p[1].x*m[1]+p[2].x*m[2]+p[3].x*m[3])/6.0f;
	ret->y = (p[0].y*m[0]+p[1].y*m[1]+p[2].y*m[2]+p[3].y*m[3])/6.0f;
	ret->z = (p[0].z*m[0]+p[1].z*m[1]+p[2].z*m[2]+p[3].z*m[3])/6.0f;
}

#define beta1 1.0f
#define beta2 0.8f

void spline3( float t, Fvector *p, Fvector *ret )
{
	float	s= 1.0f - t;
	float   t2 = t * t;
	float   t3 = t2 * t;
	float	b12=beta1*beta2;
	float	b13=b12*beta1;
	float	delta=2.0f-b13+4.0f*b12+4.0f*beta1+beta2+2.0f;
	float	d=1.0f/delta;
	float	b0=2.0f*b13*d*s*s*s;
	float	b3=2.0f*t3*d;
	float	b1=d*(2*b13*t*(t2-3*t+3)+2*b12*(t3-3*t2+2)+2*beta1*(t3-3*t+2)+beta2*(2*t3-3*t2+1));
	float	b2=d*(2*b12*t2*(-t+3)+2*beta1*t*(-t2+3)+beta2*t2*(-2*t+3)+2*(-t3+1));

	ret->x = p[0].x*b0+p[1].x*b1+p[2].x*b2+p[3].x*b3;
	ret->y = p[0].y*b0+p[1].y*b1+p[2].y*b2+p[3].y*b3;
	ret->z = p[0].z*b0+p[1].z*b1+p[2].z*b2+p[3].z*b3;
}

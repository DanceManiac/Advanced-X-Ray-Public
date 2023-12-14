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
}


#ifdef _WIN32
static const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;     // Must be 0x1000.
	LPCSTR szName;    // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static void set_thread_name(DWORD dwThreadID, const char* threadName)
{
	// DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

void set_current_thread_name(const char* threadName)
{
	set_thread_name(GetCurrentThreadId(), threadName);
}

void set_thread_name(const char* threadName, std::thread& thread)
{
	DWORD threadId = ::GetThreadId(static_cast<HANDLE>(thread.native_handle()));
	set_thread_name(threadId, threadName);
}
#else
void set_thread_name(const char* threadName, std::thread& thread)
{
	auto handle = thread.native_handle();
	pthread_setname_np(handle, threadName);
}

#include <sys/prctl.h>
void set_current_thread_name(const char* threadName) { prctl(PR_SET_NAME, threadName, 0, 0, 0); }

#endif


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
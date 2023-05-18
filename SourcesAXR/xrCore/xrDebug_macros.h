#ifndef xrDebug_macrosH
#define xrDebug_macrosH
#pragma once

//#define ANONYMOUS_BUILD

#ifndef __BORLANDC__
#	ifndef ANONYMOUS_BUILD
#		define DEBUG_INFO				__FILE__,__LINE__,__FUNCTION__
#	else // ANONYMOUS_BUILD
#		define DEBUG_INFO				"",__LINE__,""
#	endif // ANONYMOUS_BUILD
#else // __BORLANDC__
#	define DEBUG_INFO					__FILE__,__LINE__,__FILE__
#endif // __BORLANDC__

#ifdef ANONYMOUS_BUILD
	#define _TRE(arg)	""
#else
	#define _TRE(arg)	arg
#endif


#	define CHECK_OR_EXIT(expr,message)	do {if (!(expr)) ::Debug.do_exit(message);} while (0)

#	define R_ASSERT(expr)				do {static bool ignore_always = false; if (!ignore_always && !(expr)) ::Debug.fail(_TRE(#expr),DEBUG_INFO,ignore_always);} while(0)
#	define R_ASSERT2(expr,e2)			do {static bool ignore_always = false; if (!ignore_always && !(expr)) ::Debug.fail(_TRE(#expr),_TRE(e2),DEBUG_INFO,ignore_always);} while(0)
#	define R_ASSERT3(expr,e2,e3)		do {static bool ignore_always = false; if (!ignore_always && !(expr)) ::Debug.fail(_TRE(#expr),_TRE(e2),_TRE(e3),DEBUG_INFO,ignore_always);} while(0)
#	define R_ASSERT4(expr,e2,e3,e4)		do {static bool ignore_always = false; if (!ignore_always && !(expr)) ::Debug.fail(_TRE(#expr),_TRE(e2),_TRE(e3),_TRE(e4),DEBUG_INFO,ignore_always);} while(0)
#	define R_CHK(expr)					do {static bool ignore_always = false; HRESULT hr = expr; if (!ignore_always && FAILED(hr)) ::Debug.error(hr,_TRE(#expr),DEBUG_INFO,ignore_always);} while(0)
#	define R_CHK2(expr,e2)				do {static bool ignore_always = false; HRESULT hr = expr; if (!ignore_always && FAILED(hr)) ::Debug.error(hr,_TRE(#expr),_TRE(e2),DEBUG_INFO,ignore_always);} while(0)
#	define FATAL(description)			Debug.fatal(DEBUG_INFO,description)

#define R_ASSERT1_CURE(expr, can_be_cured, cure)\
    do\
    {\
        static bool ignoreAlways = false;\
        if (!ignoreAlways && !(expr))\
        {\
            if (!can_be_cured)\
                xrDebug::Fail(ignoreAlways, DEBUG_INFO, #expr);\
            else\
            {\
                cure;\
            }\
        }\
    } while (false)

#define R_ASSERT2_CURE(expr, desc, can_be_cured, cure)\
    do\
    {\
        static bool ignoreAlways = false;\
        if (!ignoreAlways && !(expr))\
        {\
            if (!can_be_cured)\
                xrDebug::Fail(ignoreAlways, DEBUG_INFO, #expr, desc);\
            else\
            {\
                cure;\
            }\
        }\
    } while (false)

#define R_ASSERT3_CURE(expr, desc, arg1, can_be_cured, cure)\
    do\
    {\
        static bool ignoreAlways = false;\
        if (!ignoreAlways && !(expr))\
        {\
            if (!can_be_cured)\
                xrDebug::Fail(ignoreAlways, DEBUG_INFO, #expr, desc, arg1);\
            else\
            {\
                cure;\
            }\
        }\
    } while (false)

#define R_ASSERT4_CURE(expr, cure, desc, arg1, arg2, can_be_cured)\
    do\
    {\
        static bool ignoreAlways = false;\
        if (!ignoreAlways && !(expr))\
        {\
            if (!can_be_cured)\
                xrDebug::Fail(ignoreAlways, DEBUG_INFO, #expr, desc, arg1, arg2);\
            else\
            {\
                cure;\
            }\
        }\
    } while (false)

#	ifdef VERIFY
#		undef VERIFY
#	endif // VERIFY

#	ifdef DEBUG
#		define NODEFAULT				FATAL("nodefault reached")
#		define VERIFY(expr)				do {static bool ignore_always = false; if (!ignore_always && !(expr)) ::Debug.fail(#expr,DEBUG_INFO,ignore_always);} while(0)
#		define VERIFY2(expr,e2)		do {static bool ignore_always = false; if (!ignore_always && !(expr)) ::Debug.fail(#expr,e2,DEBUG_INFO,ignore_always);} while(0)
#		define VERIFY3(expr,e2,e3)	do {static bool ignore_always = false; if (!ignore_always && !(expr)) ::Debug.fail(#expr,e2,e3,DEBUG_INFO,ignore_always);} while(0)
#		define VERIFY4(expr,e2,e3,e4)do {static bool ignore_always = false; if (!ignore_always && !(expr)) ::Debug.fail(#expr,e2,e3,e4,DEBUG_INFO,ignore_always);} while(0)
#		define CHK_DX(expr)				do {static bool ignore_always = false; HRESULT hr = expr; if (!ignore_always && FAILED(hr)) ::Debug.error(hr,#expr,DEBUG_INFO,ignore_always);} while(0)
#		define GAME_PAUSE(a,b,c,msg) Device.Pause(a,b,c,msg)
#	else // DEBUG
#		ifdef __BORLANDC__
#			define NODEFAULT
#		else
#			define NODEFAULT __assume(0)
#		endif
#		define VERIFY(expr)
#		define VERIFY2(expr, e2)
#		define VERIFY3(expr, e2, e3)
#		define VERIFY4(expr, e2, e3, e4)
#		define CHK_DX(a) a
#		define GAME_PAUSE(a,b,c,msg) Device.Pause(a,b,c)
#	endif // DEBUG
//---------------------------------------------------------------------------------------------
// FIXMEs / TODOs / NOTE macros
//---------------------------------------------------------------------------------------------
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

#define NOTE( x )  message( x )
#define FILE_LINE  message( __FILE__LINE__ )

#define TODO( x )  message( __FILE__LINE__"\n"           \
	" ------------------------------------------------\n" \
	"|  TODO :   " #x "\n" \
	" -------------------------------------------------\n" )
#define FIXME( x )  message(  __FILE__LINE__"\n"           \
	" ------------------------------------------------\n" \
	"|  FIXME :  " #x "\n" \
	" -------------------------------------------------\n" )
#define todo( x )  message( __FILE__LINE__" TODO :   " #x "\n" ) 
#define fixme( x )  message( __FILE__LINE__" FIXME:   " #x "\n" ) 

//--------- static assertion
template<bool>	struct CompileTimeError;
template<>		struct CompileTimeError<true>	{};
#define STATIC_CHECK(expr, msg) \
{ \
	CompileTimeError<((expr) != 0)> ERROR_##msg; \
	(void)ERROR_##msg; \
}
#endif // xrDebug_macrosH
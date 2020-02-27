
// (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
// Permission to copy, use, modify, sell and distribute this software is 
// granted provided this copyright notice appears in all copies. This software 
// is provided "as is" without express or implied warranty, and with no claim 
// as to its suitability for any purpose.
//
// See http://www.boost.org for most recent version including documentation.

#ifndef BOOST_TT_IS_INTEGRAL_HPP_INCLUDED
#define BOOST_TT_IS_INTEGRAL_HPP_INCLUDED

#include "boost/config.hpp"

// should be the last #include
#include "boost/type_traits/detail/bool_trait_def.hpp"

namespace boost {

//* is a type T an [cv-qualified-] integral type described in the standard (3.9.1p3)
// as an extention we include long long, as this is likely to be added to the
// standard at a later date
BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_integral,T,false)

BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned char,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned short,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned int,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned long,true)

BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed char,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed short,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed int,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed long,true)

BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,bool,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,char,true)

#ifndef BOOST_NO_INTRINSIC_WCHAR_T
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,wchar_t,true)
#endif

#if (defined(BOOST_MSVC) && (BOOST_MSVC == 1200)) || (defined(BOOST_INTEL_CXX_VERSION) && defined(_MSC_VER) && (BOOST_INTEL_CXX_VERSION <= 600))
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int8,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int8,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int16,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int16,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int32,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int32,true)
#endif

# if defined(BOOST_HAS_LONG_LONG)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned long long,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,long long,true)
#elif defined(BOOST_HAS_MS_INT64)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int64,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int64,true)
#endif

} // namespace boost

#include "boost/type_traits/detail/bool_trait_undef.hpp"

#endif // BOOST_TT_IS_INTEGRAL_HPP_INCLUDED

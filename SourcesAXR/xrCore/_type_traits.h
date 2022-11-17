#ifndef _STL_EXT_type_traits
#define _STL_EXT_type_traits
#pragma once

// 1. class or not class
template<typename T>
struct is_class
{
    struct _yes { char _a[1]; };
    struct _no { char _a[2]; };

    template <class U> static _yes is_class_tester(void(U::*)(void));
    template <class U> static _no is_class_tester(...);

    enum { result = (sizeof(_yes) == sizeof(is_class_tester<T>(0))) };
};

#endif

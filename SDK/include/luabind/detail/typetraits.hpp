// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <luabind/config.hpp>
#include <luabind/detail/primitives.hpp>

#include <type_traits>

namespace luabind { 
	namespace detail
	{
		template<typename T>
		struct is_const_reference : public std::integral_constant<bool, std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>>
		{
		};

		template<typename T>
		struct is_nonconst_reference : public std::integral_constant<bool, std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>>
		{
		};

		template<typename T>
		struct is_const_pointer : public std::integral_constant<bool, std::is_pointer_v<T> && std::is_const_v<std::remove_pointer_t<T>>>
		{
		};


		template<typename T>
		struct is_nonconst_pointer : public std::integral_constant<bool, std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>>>
		{
		};
	}

	struct null_type {};

	template<typename T>
	struct is_null_type : public std::false_type {};

	template< >
	struct is_null_type< null_type > : public std::true_type {};
}

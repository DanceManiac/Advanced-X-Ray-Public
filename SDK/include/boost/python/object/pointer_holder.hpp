#if !defined(BOOST_PP_IS_ITERATING)

// Copyright David Abrahams 2001. Permission to copy, use,
// modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

# ifndef POINTER_HOLDER_DWA20011215_HPP
#  define POINTER_HOLDER_DWA20011215_HPP 

#  include <boost/type.hpp>

#  include <boost/python/instance_holder.hpp>
#  include <boost/python/type_id.hpp>
#  include <boost/python/object/inheritance.hpp>
#  include <boost/python/object/forward.hpp>
#  include <boost/python/pointee.hpp>
#  include <boost/python/detail/force_instantiate.hpp>
#  include <boost/python/detail/preprocessor.hpp>

#  include <boost/mpl/if.hpp>
#  include <boost/mpl/apply.hpp>

#  include <boost/preprocessor/comma_if.hpp>
#  include <boost/preprocessor/iterate.hpp>
#  include <boost/preprocessor/repeat.hpp>
#  include <boost/preprocessor/debug/line.hpp>
#  include <boost/preprocessor/enum_params.hpp>
#  include <boost/preprocessor/repetition/enum_binary_params.hpp>

namespace boost { namespace python { namespace objects {

template <class T>
bool is_null(T const& p, ...)
{
    return p.get() == 0;
}

template <class T>
bool is_null(T* p, int)
{
    return p == 0;
}

#  define BOOST_PYTHON_UNFORWARD_LOCAL(z, n, _) BOOST_PP_COMMA_IF(n) (typename unforward<A##n>::type)(a##n)

template <class Pointer, class Value>
struct pointer_holder : instance_holder
{
    typedef Value value_type;
    
    pointer_holder(Pointer);

    // Forward construction to the held object

#  define BOOST_PP_ITERATION_PARAMS_1 (4, (0, BOOST_PYTHON_MAX_ARITY, <boost/python/object/pointer_holder.hpp>, 1))
#  include BOOST_PP_ITERATE()

 private: // types
    
 private: // required holder implementation
    void* holds(type_info);

 private: // data members
    Pointer m_p;
};

template <class Pointer, class Value>
struct pointer_holder_back_reference : instance_holder
{
 private:
    typedef typename python::pointee<Pointer>::type held_type;
 public:
    typedef Value value_type;

    // Not sure about this one -- can it work? The source object
    // undoubtedly does not carry the correct back reference pointer.
    pointer_holder_back_reference(Pointer);

    // Forward construction to the held object
#  define BOOST_PP_ITERATION_PARAMS_1 (4, (0, BOOST_PYTHON_MAX_ARITY, <boost/python/object/pointer_holder.hpp>, 2))
#  include BOOST_PP_ITERATE()

 private: // required holder implementation
    void* holds(type_info);

 private: // data members
    Pointer m_p;
};

#  undef BOOST_PYTHON_UNFORWARD_LOCAL

template <class Pointer, class Value>
inline pointer_holder<Pointer,Value>::pointer_holder(Pointer p)
    : m_p(p)
{
}

template <class Pointer, class Value>
inline pointer_holder_back_reference<Pointer,Value>::pointer_holder_back_reference(Pointer p)
    : m_p(p)
{
}

template <class Pointer, class Value>
void* pointer_holder<Pointer, Value>::holds(type_info dst_t)
{
    if (dst_t == python::type_id<Pointer>())
        return &this->m_p;

    if (objects::is_null(this->m_p, 0))
        return 0;
    
    type_info src_t = python::type_id<Value>();
    Value* p = &*this->m_p;
    return src_t == dst_t ? p : find_dynamic_type(p, src_t, dst_t);
}

template <class Pointer, class Value>
void* pointer_holder_back_reference<Pointer, Value>::holds(type_info dst_t)
{
    if (dst_t == python::type_id<Pointer>())
        return &this->m_p;

    if (objects::is_null(this->m_p, 0))
        return 0;
    
    if (dst_t == python::type_id<held_type>())
        return &*this->m_p;

    type_info src_t = python::type_id<Value>();
    Value* p = &*this->m_p;
    return src_t == dst_t ? p : find_dynamic_type(p, src_t, dst_t);
}

}}} // namespace boost::python::objects

# endif // POINTER_HOLDER_DWA20011215_HPP

/* --------------- pointer_holder --------------- */
#elif BOOST_PP_ITERATION_DEPTH() == 1 && BOOST_PP_ITERATION_FLAGS() == 1
# line BOOST_PP_LINE(__LINE__, pointer_holder.hpp)

# define N BOOST_PP_ITERATION()

# if (N != 0)
    template< BOOST_PP_ENUM_PARAMS_Z(1, N, class A) >
# endif
    pointer_holder(PyObject* BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, A, a))
        : m_p(new Value(
                BOOST_PP_REPEAT_1ST(N, BOOST_PYTHON_UNFORWARD_LOCAL, nil)
            ))
    {}

# undef N

/* --------------- pointer_holder_back_reference --------------- */
#elif BOOST_PP_ITERATION_DEPTH() == 1 && BOOST_PP_ITERATION_FLAGS() == 2
# line BOOST_PP_LINE(__LINE__, pointer_holder.hpp(pointer_holder_back_reference))

# define N BOOST_PP_ITERATION()

# if (N != 0)
    template < BOOST_PP_ENUM_PARAMS_Z(1, N, class A) >
# endif
    pointer_holder_back_reference(
        PyObject* p BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, A, a))
        : m_p(new held_type(
                    p BOOST_PP_COMMA_IF(N) BOOST_PP_REPEAT_1ST(N, BOOST_PYTHON_UNFORWARD_LOCAL, nil)
            ))
    {}

# undef N

#endif

#ifndef _STL_EXT_internal
#define _STL_EXT_internal

using std::swap;

#include "_type_traits.h"

#include <string>
#include <vector>
#include <list>
#include <set>
#include "_types.h"
#include "_rect.h"
#include "_plane.h"
#include "_vector2.h"
#include "_vector3d.h"
#include "_color.h"
//#include "_std_extensions.h"
#include "xrMemory.h"
#include "xalloc.h"
#include "xr_vector.h"
#include "xr_map.h"
#include "xr_set.h"
#include "xr_stack.h"
#include "xr_list.h"
#include "xr_deque.h"
#include "xr_string.h"
#include "xr_unordered_map.h"

#ifdef STLPORT

namespace std
{
    template<class _Tp1, class _Tp2>
    inline xalloc<_Tp2>& __stl_alloc_rebind(xalloc<_Tp1>& __a, const _Tp2*)
    {
        return (xalloc<_Tp2>&)(__a);
    }
    template<class _Tp1, class _Tp2>
    inline xalloc<_Tp2> __stl_alloc_create(xalloc<_Tp1>&, const _Tp2*)
    {
        return xalloc<_Tp2>();
    }
}

template <typename V, class _HashFcn = std::hash<V>, class _EqualKey = std::equal_to<V>, typename allocator = xalloc<V> > class xr_hash_set : public std::hash_set < V, _HashFcn, _EqualKey, allocator > { public: u32 size() const { return (u32)__super::size(); } };
template <typename V, class _HashFcn = std::hash<V>, class _EqualKey = std::equal_to<V>, typename allocator = xalloc<V> > class xr_hash_multiset : public std::hash_multiset < V, _HashFcn, _EqualKey, allocator > { public: u32 size() const { return (u32)__super::size(); } };

template <typename K, class V, class _HashFcn = std::hash<K>, class _EqualKey = std::equal_to<K>, typename allocator = xalloc<std::pair<K, V> > > class xr_hash_map : public std::hash_map < K, V, _HashFcn, _EqualKey, allocator > { public: u32 size() const { return (u32)__super::size(); } };
template <typename K, class V, class _HashFcn = std::hash<K>, class _EqualKey = std::equal_to<K>, typename allocator = xalloc<std::pair<K, V> > > class xr_hash_multimap : public std::hash_multimap < K, V, _HashFcn, _EqualKey, allocator > { public: u32 size() const { return (u32)__super::size(); } };
#else
#ifndef _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#endif
#include <hash_map>
template <typename K, class V, class _Traits = stdext::hash_compare<K, std::less<K>>,
    typename allocator = xalloc<std::pair<const K, V>>>
    class xr_hash_map : public stdext::hash_map<K, V, _Traits, allocator>
{
public:
    u32 size() const { return (u32) __super ::size(); }
};
#endif // #ifdef STLPORT

#endif

#include "predicates.h"

// STL extensions
#define DEFINE_SVECTOR(T,C,N,I) typedef svector< T, C > N; typedef N::iterator I;

#include "FixedVector.h"
#include "buffer_vector.h"
#include "xr_vector_defs.h"

#pragma once
#include <unordered_map>
#include "xalloc.h"

template <typename K, class V, class Hasher = std::hash<K>, class Traits = std::equal_to<K>,
    typename allocator = xr_allocator<std::pair<const K, V>>>
    using xr_unordered_map = std::unordered_map<K, V, Hasher, Traits, allocator>;
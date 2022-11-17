#pragma once


#define DECLARE_MT_LOCK(lock) xrCriticalSection lock
#define DECLARE_MT_SCOPE_LOCK(lock) ScopeLock scope(&lock); UNUSED(scope)

#define DO_MT_LOCK(lock) lock.Enter()
#define DO_MT_UNLOCK(lock) lock.Leave()
#define DO_MT_PROCESS_RANGE(range, function) tbb::parallel_for_each(range, function)

#define FOR_START(type, start, finish, counter)\
tbb::parallel_for(tbb::blocked_range<type>(start, finish), [&](const tbb::blocked_range<type>& range) {\
    for (type counter = range.begin(); counter != range.end(); ++counter)

#define FOR_END });
#define ACCELERATED_SORT tbb::parallel_sort
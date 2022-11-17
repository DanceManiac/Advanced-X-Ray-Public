#pragma once

class xrCriticalSection;

class XRCORE_API ScopeLock
{
    xrCriticalSection* syncObject;

public:
    ScopeLock(xrCriticalSection* SyncObject);
    ~ScopeLock();
};
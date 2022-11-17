#include "stdafx.h"
#include "ScopeLock.h"
#include "xrSyncronize.h"
#include "xrDebug.h"

ScopeLock::ScopeLock(xrCriticalSection* SyncObject) : syncObject(SyncObject)
{
    R_ASSERT(syncObject);
    syncObject->Enter();
}

ScopeLock::~ScopeLock()
{
    syncObject->Leave();
}
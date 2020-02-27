// Magic Software, Inc.
// http://www.magic-software.com
// Copyright (c) 2000-2002.  All Rights Reserved
//
// Source code from Magic Software is supplied under the terms of a license
// agreement and may not be copied or disclosed except in accordance with the
// terms of that agreement.  The various license agreements may be found at
// the Magic Software web site.  This file is subject to the license
//
// FREE SOURCE CODE
// http://www.magic-software.com/License/free.pdf

#include "MgcAppr3DSphereFit.h"
using namespace Mgc;

//----------------------------------------------------------------------------
bool Mgc::SphereFit (int iQuantity, const Vector3* akPoint, Vector3& rkCenter,
    Real& rfRadius)
{
    // user-selected parameters
    const int iMaxIterations = 64;
    const Real fTolerance = 1e-06f;

    // compute the average of the data points
    Vector3 kAverage = akPoint[0];
    int i0;
    for (i0 = 1; i0 < iQuantity; i0++)
        kAverage += akPoint[i0];
    Real fInvQuantity = 1.0f/iQuantity;
    kAverage *= fInvQuantity;

    // initial guess
    rkCenter = kAverage;

    int i1;
    for (i1 = 0; i1 < iMaxIterations; i1++)
    {
        // update the iterates
        Vector3 kCurrent = rkCenter;

        // compute average L, dL/da, dL/db, dL/dc
        Real fLAverage = 0.0f;
        Vector3 kDerLAverage = Vector3::ZERO;
        for (i0 = 0; i0 < iQuantity; i0++)
        {
            Vector3 kDiff = akPoint[i0] - rkCenter;
            Real fLength = kDiff.Length();
            if ( fLength > fTolerance )
            {
                fLAverage += fLength;
                Real fInvLength = 1.0f/fLength;
                kDerLAverage -= fInvLength*kDiff;
            }
        }
        fLAverage *= fInvQuantity;
        kDerLAverage *= fInvQuantity;

        rkCenter = kAverage + fLAverage*kDerLAverage;
        rfRadius = fLAverage;

        if ( Math::FAbs(rkCenter.x - kCurrent.x) <= fTolerance
        &&   Math::FAbs(rkCenter.y - kCurrent.y) <= fTolerance
        &&   Math::FAbs(rkCenter.z - kCurrent.z) <= fTolerance )
        {
            break;
        }
    }

    return i1 < iMaxIterations;
}
//----------------------------------------------------------------------------



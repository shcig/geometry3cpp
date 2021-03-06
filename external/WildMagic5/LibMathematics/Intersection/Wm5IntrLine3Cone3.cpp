// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2010/10/01)

#include "Wm5MathematicsPCH.h"
#include "Wm5IntrLine3Cone3.h"

namespace Wm5
{
//----------------------------------------------------------------------------
template <typename Real>
IntrLine3Cone3<Real>::IntrLine3Cone3 (const Line3<Real>& line,
    const Cone3<Real>& cone)
    :
    mLine(&line),
    mCone(&cone)
{
}
//----------------------------------------------------------------------------
template <typename Real>
const Line3<Real>& IntrLine3Cone3<Real>::GetLine () const
{
    return *mLine;
}
//----------------------------------------------------------------------------
template <typename Real>
const Cone3<Real>& IntrLine3Cone3<Real>::GetCone () const
{
    return *mCone;
}
//----------------------------------------------------------------------------
template <typename Real>
bool IntrLine3Cone3<Real>::Find ()
{
    // Set up the quadratic Q(t) = c2*t^2 + 2*c1*t + c0 that corresponds to
    // the cone.  Let the vertex be V, the unit-length direction vector be A,
    // and the angle measured from the cone axis to the cone wall be Theta,
    // and define g = cos(Theta).  A point X is on the cone wall whenever
    // Dot(A,(X-V)/|X-V|) = g.  Square this equation and factor to obtain
    //   (X-V)^T * (A*A^T - g^2*I) * (X-V) = 0
    // where the superscript T denotes the transpose operator.  This defines
    // a double-sided cone.  The line is L(t) = P + t*D, where P is the line
    // origin and D is a unit-length direction vector.  Substituting
    // X = L(t) into the cone equation above leads to Q(t) = 0.  Since we
    // want only intersection points on the single-sided cone that lives in
    // the half-space pointed to by A, any point L(t) generated by a root of
    // Q(t) = 0 must be tested for Dot(A,L(t)-V) >= 0.
    Real AdD = mCone->Axis.Dot(mLine->Direction);
    Real cosSqr = mCone->CosAngle*mCone->CosAngle;
    Vector3<Real> E = mLine->Origin - mCone->Vertex;
    Real AdE = mCone->Axis.Dot(E);
    Real DdE = mLine->Direction.Dot(E);
    Real EdE = E.Dot(E);
    Real c2 = AdD*AdD - cosSqr;
    Real c1 = AdD*AdE - cosSqr*DdE;
    Real c0 = AdE*AdE - cosSqr*EdE;
    Real dot;

    // Solve the quadratic.  Keep only those X for which Dot(A,X-V) >= 0.
    if (Math<Real>::FAbs(c2) >= Math<Real>::ZERO_TOLERANCE)
    {
        // c2 != 0
        Real discr = c1*c1 - c0*c2;
        if (discr < (Real)0)
        {
            // Q(t) = 0 has no real-valued roots.  The line does not
            // intersect the double-sided cone.
            mIntersectionType = IT_EMPTY;
            mQuantity = 0;
        }
        else if (discr > Math<Real>::ZERO_TOLERANCE)
        {
            // Q(t) = 0 has two distinct real-valued roots.  However, one or
            // both of them might intersect the portion of the double-sided
            // cone "behind" the vertex.  We are interested only in those
            // intersections "in front" of the vertex.
            Real root = Math<Real>::Sqrt(discr);
            Real invC2 = ((Real)1)/c2;
            mQuantity = 0;

            Real t = (-c1 - root)*invC2;
            mPoint[mQuantity] = mLine->Origin + t*mLine->Direction;
            E = mPoint[mQuantity] - mCone->Vertex;
            dot = E.Dot(mCone->Axis);
            if (dot > (Real)0)
            {
                mQuantity++;
            }

            t = (-c1 + root)*invC2;
            mPoint[mQuantity] = mLine->Origin + t*mLine->Direction;
            E = mPoint[mQuantity] - mCone->Vertex;
            dot = E.Dot(mCone->Axis);
            if (dot > (Real)0)
            {
                mQuantity++;
            }

            if (mQuantity == 2)
            {
                // The line intersects the single-sided cone in front of the
                // vertex twice.
                mIntersectionType = IT_SEGMENT;
            }
            else if (mQuantity == 1)
            {
                // The line intersects the single-sided cone in front of the
                // vertex once.  The other intersection is with the
                // single-sided cone behind the vertex.
                mIntersectionType = IT_RAY;
                mPoint[mQuantity++] = mLine->Direction;
            }
            else
            {
                // The line intersects the single-sided cone behind the vertex
                // twice.
                mIntersectionType = IT_EMPTY;
            }
        }
        else
        {
            // One repeated real root (line is tangent to the cone).
            mPoint[0] = mLine->Origin - (c1/c2)*mLine->Direction;
            E = mPoint[0] - mCone->Vertex;
            if (E.Dot(mCone->Axis) > (Real)0)
            {
                mIntersectionType = IT_POINT;
                mQuantity = 1;
            }
            else
            {
                mIntersectionType = IT_EMPTY;
                mQuantity = 0;
            }
        }
    }
    else if (Math<Real>::FAbs(c1) >= Math<Real>::ZERO_TOLERANCE)
    {
        // c2 = 0, c1 != 0 (D is a direction vector on the cone boundary)
        mPoint[0] = mLine->Origin - (((Real)0.5)*c0/c1)*mLine->Direction;
        E = mPoint[0] - mCone->Vertex;
        dot = E.Dot(mCone->Axis);
        if (dot > (Real)0)
        {
            mIntersectionType = IT_RAY;
            mQuantity = 2;
            mPoint[1] = mLine->Direction;
        }
        else
        {
            mIntersectionType = IT_EMPTY;
            mQuantity = 0;
        }
    }
    else if (Math<Real>::FAbs(c0) >= Math<Real>::ZERO_TOLERANCE)
    {
        // c2 = c1 = 0, c0 != 0
        mIntersectionType = IT_EMPTY;
        mQuantity = 0;
    }
    else
    {
        // c2 = c1 = c0 = 0, cone contains ray V+t*D where V is cone vertex
        // and D is the line direction.
        mIntersectionType = IT_RAY;
        mQuantity = 2;
        mPoint[0] = mCone->Vertex;
        mPoint[1] = mLine->Direction;
    }

    return mIntersectionType != IT_EMPTY;
}
//----------------------------------------------------------------------------
template <typename Real>
int IntrLine3Cone3<Real>::GetQuantity () const
{
    return mQuantity;
}
//----------------------------------------------------------------------------
template <typename Real>
const Vector3<Real>& IntrLine3Cone3<Real>::GetPoint (int i) const
{
    return mPoint[i];
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Explicit instantiation.
//----------------------------------------------------------------------------
template WM5_MATHEMATICS_ITEM
class IntrLine3Cone3<float>;

template WM5_MATHEMATICS_ITEM
class IntrLine3Cone3<double>;
//----------------------------------------------------------------------------
}

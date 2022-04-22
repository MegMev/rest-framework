/******************** REST disclaimer ***********************************
 * This file is part of the REST software framework.                     *
 *                                                                       *
 * Copyright (C) 2016 GIFNA/TREX (University of Zaragoza)                *
 * For more information see http://gifna.unizar.es/trex                  *
 *                                                                       *
 * REST is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * REST is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have a copy of the GNU General Public License along with   *
 * REST in $REST_PATH/LICENSE.                                           *
 * If not, see http://www.gnu.org/licenses/.                             *
 * For the list of contributors see $REST_PATH/CREDITS.                  *
 *************************************************************************/

#include "TRestPhysics.h"

#include <iostream>
#include <limits>

using namespace std;

//////////////////////////////////////////////////////////////////////////
///
/// The methods and physics constants in this class can be accessed using the
/// REST_Physics:: namespace.
///
/// We include basic geometrical operations that transform space coordinates
/// or implement vector transportation.
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2019-Mar:   First concept and implementation of REST_Physics namespace.
/// \author     Javier Galan <javier.galan@unizar.es>
///
/// \namespace REST_Physics
///
/// <hr>
namespace REST_Physics {

///////////////////////////////////////////////
/// \brief This method will translate the vector with direction `dir` starting at position `pos` to the plane
/// defined by the normal vector plane, `n` that contains the point `a` in the plane.
///
/// If the vector is parallel to the plane the position `pos` will not be translated.
///
TVector3 MoveToPlane(TVector3 pos, TVector3 dir, TVector3 n, TVector3 a) {
    if (n * dir != 0) {
        Double_t t = (n * a - n * pos) / (n * dir);

        return pos + t * dir;
    }
    return pos;
}

///////////////////////////////////////////////
/// \brief This method will return the distance from `point` to the straight defined by `axisPoint` and
/// `axisVector`.
///
Double_t DistanceToAxis(const TVector3& axisPoint, const TVector3& axisVector, const TVector3& point) {
    TVector3 a = axisVector.Cross(axisPoint - point);
    return a.Mag() / axisVector.Mag();
}

///////////////////////////////////////////////
/// \brief This method will find the intersection of the trajectory defined by the vector starting at `pos`
/// and  moving in direction `dir` and the plane defined by its normal vector `n` and the point `a`. This is
/// equivalent to move/translate the position `pos` to the plane.
///
TVector3 GetPlaneVectorIntersection(const TVector3& pos, const TVector3& dir, const TVector3& n,
                                    const TVector3& a) {
    return MoveToPlane(pos, dir, n, a);
}

///////////////////////////////////////////////
/// \brief It returns the cone matrix M = d^T x d - cosTheta^2 x I, extracted from the document
/// by "David Eberly, Geometric Tools, Redmond WA 98052, Intersection of a Line and a Cone".
///
TMatrixD GetConeMatrix(const TVector3& d, const Double_t& cosTheta) {
    double cAxis[3];
    d.GetXYZ(cAxis);

    TVectorD coneAxis(3, cAxis);

    TMatrixD M(3, 3);
    M.Rank1Update(coneAxis, coneAxis);

    double cT2 = cosTheta * cosTheta;
    TMatrixD gamma(3, 3);
    gamma.UnitMatrix();
    gamma *= cT2;

    M -= gamma;
    return M;
}

///////////////////////////////////////////////
/// \brief This method will find the intersection of the trajectory defined by the vector starting at
/// `pos` and  moving in direction `dir` and the cone defined by its axis vector `d` and the vertex`v`.
/// The cosine of the angle defining the cone should be also given inside the `cosTheta` argument.
///
/// This method will return `t`, which is the value the particle position, `pos`, needs to be displaced
/// by the vector, `dir`, to get the particle at the surface of the cone. If the particle does not
/// cross the cone, then the value returned will be zero (no particle displacement).
//
/// This method is based on the document by "David Eberly, Geometric Tools, Redmond WA 98052,
/// Intersection of a Line and a Cone".
///
Double_t GetConeVectorIntersection(const TVector3& pos, const TVector3& dir, const TVector3& d,
                                   const TVector3& v, const Double_t& cosTheta) {
    TMatrixD M = GetConeMatrix(d, cosTheta);
    return GetConeVectorIntersection(pos, dir, M, d, v);
}

///////////////////////////////////////////////
/// \brief This method will find the intersection of the trajectory defined by the vector starting at `pos`
/// and moving in direction `dir` and the cone defined by its characteristic matrix `M`, which is built
/// using the cone axis vector `d` as `d^T x d`,  and the vertex`v`. The resulting TVector3 will be the
/// position of the particle placed at the cone surface.
///
/// This method will return `t`, which is the value the particle position, `pos`, needs to be displaced
/// by the vector, `dir`, to get the particle at the surface of the cone. If the particle does not
/// cross the cone, then the value returned will be zero (no particle displacement).
///
/// This method is based on the document by "David Eberly, Geometric Tools, Redmond WA 98052,
/// Intersection of a Line and a Cone".
///
Double_t GetConeVectorIntersection(const TVector3& pos, const TVector3& dir, const TMatrixD& M,
                                   const TVector3& axis, const TVector3& v) {
    double u[3];
    dir.GetXYZ(u);
    TMatrixD U(3, 1, u);
    TMatrixD Ut(1, 3, u);

    double delta[3];
    TVector3 deltaV = pos - v;
    deltaV.GetXYZ(delta);
    TMatrixD D(3, 1, delta);
    TMatrixD Dt(1, 3, delta);

    TMatrixD C2 = Ut * M * U;
    Double_t c2 = C2[0][0];

    TMatrixD C1 = Ut * M * D;
    Double_t c1 = C1[0][0];

    TMatrixD C0 = Dt * M * D;
    Double_t c0 = C0[0][0];

    Double_t root = c1 * c1 - c0 * c2;
    if (root < 0) return 0;

    Double_t t1 = (-c1 + TMath::Sqrt(root)) / c2;
    Double_t t2 = (-c1 - TMath::Sqrt(root)) / c2;

    // The projections along the cone axis. If positive then the solution
    // gives the cone intersection with the side defined by `axis`
    Double_t h1 = t1 * dir.Dot(axis) + axis.Dot(deltaV);
    Double_t h2 = t2 * dir.Dot(axis) + axis.Dot(deltaV);

    // We use it to select the root we are interested in
    if (h2 > 0)
        return t2;
    else
        return t1;
}

///////////////////////////////////////////////
/// \brief This method transports a position `pos` by a distance `d` in the direction defined by `dir`.
///
TVector3 MoveByDistance(TVector3 pos, TVector3 dir, Double_t d) { return pos + d * dir.Unit(); }

///////////////////////////////////////////////
/// \brief This method transports a position `pos` by a distance `d` in the direction defined by `dir`.
/// This method assumes the vector `dir` is unitary!
///
TVector3 MoveByDistanceFast(TVector3 pos, TVector3 dir, Double_t d) { return pos + d * dir; }

///////////////////////////////////////////////
/// \brief This method returns the cartesian distance between vector v2 and v1.
///
Double_t GetDistance(TVector3 v1, TVector3 v2) { return (v2 - v1).Mag(); }

///////////////////////////////////////////////
/// \brief This method returns the squared cartesian distance between vector v2 and v1.
///
Double_t GetDistance2(TVector3 v1, TVector3 v2) { return (v2 - v1).Mag2(); }
}  // namespace REST_Physics

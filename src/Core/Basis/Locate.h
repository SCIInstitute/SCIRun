/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   Author:          Martin Cole, Frank B. Sachse
   Date:            October 8 2005
*/


#ifndef CORE_BASIS_LOCATE_H
#define CORE_BASIS_LOCATE_H 1

#include <cmath>

#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Containers/StackVector.h>
#include <Core/Utils/Exception.h>

#include <Core/Basis/share.h>

namespace SCIRun {

  template<class T>
    inline T InverseMatrix3x3(const T *p, T *q)
  {
    const T a=p[0], b=p[1], c=p[2];
    const T d=p[3], e=p[4], f=p[5];
    const T g=p[6], h=p[7], i=p[8];

    const T detp=a*e*i-c*e*g+b*f*g+c*d*h-a*f*h-b*d*i;
    const T detinvp=(detp ? 1.0/detp : 0);

    q[0]=(e*i-f*h)*detinvp;
    q[1]=(c*h-b*i)*detinvp;
    q[2]=(b*f-c*e)*detinvp;
    q[3]=(f*g-d*i)*detinvp;
    q[4]=(a*i-c*g)*detinvp;
    q[5]=(c*d-a*f)*detinvp;
    q[6]=(d*h-e*g)*detinvp;
    q[7]=(b*g-a*h)*detinvp;
    q[8]=(a*e-b*d)*detinvp;

    return detp;
  }


    /// Inline templated inverse matrix
    template <class PointVector>
    double InverseMatrix3P(const PointVector& p, double *q)
    {
      const double a=p[0].x(), b=p[0].y(), c=p[0].z();
      const double d=p[1].x(), e=p[1].y(), f=p[1].z();
      const double g=p[2].x(), h=p[2].y(), i=p[2].z();

      const double detp=a*e*i-c*e*g+b*f*g+c*d*h-a*f*h-b*d*i;
      const double detinvp=(detp ? 1.0/detp : 0);

      q[0]=(e*i-f*h)*detinvp;
      q[1]=(c*h-b*i)*detinvp;
      q[2]=(b*f-c*e)*detinvp;
      q[3]=(f*g-d*i)*detinvp;
      q[4]=(a*i-c*g)*detinvp;
      q[5]=(c*d-a*f)*detinvp;
      q[6]=(d*h-e*g)*detinvp;
      q[7]=(b*g-a*h)*detinvp;
      q[8]=(a*e-b*d)*detinvp;

      return detp;
    }

  template<class T>
    inline T DetMatrix3x3(const T *p)
  {
    const T a=p[0], b=p[1], c=p[2];
    const T d=p[3], e=p[4], f=p[5];
    const T g=p[6], h=p[7], i=p[8];

    const T detp=a*e*i-c*e*g+b*f*g+c*d*h-a*f*h-b*d*i;
    return detp;
  }

  template<class T>
    inline T ScaledDetMatrix3x3(const T *p)
  {
    const T a=p[0], b=p[1], c=p[2];
    const T d=p[3], e=p[4], f=p[5];
    const T g=p[6], h=p[7], i=p[8];

    const T detp=a*e*i-c*e*g+b*f*g+c*d*h-a*f*h-b*d*i;
    const T s = sqrt((a*a+b*b+c*c)*(d*d+e*e+f*f)*(g*g+h*h+i*i));
    return (detp/s);
  }


    /// Inline templated determinant of matrix
    template <class VectorOfPoints>
    double DetMatrix3P(const VectorOfPoints& p)
    {
      const double a=p[0].x(), b=p[0].y(), c=p[0].z();
      const double d=p[1].x(), e=p[1].y(), f=p[1].z();
      const double g=p[2].x(), h=p[2].y(), i=p[2].z();

      const double detp=a*e*i-c*e*g+b*f*g+c*d*h-a*f*h-b*d*i;
      return detp;
    }

    /// Inline templated determinant of matrix
    template <class VectorOfPoints>
    double ScaledDetMatrix3P(const VectorOfPoints& p)
    {
      const double a=p[0].x(), b=p[0].y(), c=p[0].z();
      const double d=p[1].x(), e=p[1].y(), f=p[1].z();
      const double g=p[2].x(), h=p[2].y(), i=p[2].z();

      const double detp=a*e*i-c*e*g+b*f*g+c*d*h-a*f*h-b*d*i;
      const double s = std::sqrt((a*a+b*b+c*c)*(d*d+e*e+f*f)*(g*g+h*h+i*i));
      return (detp/s);
    }

    namespace Core {
      namespace Basis {

  /// default case for volume calculation - currently not needed
  template <class VECTOR, class T>
  inline double d_volume_type(const VECTOR& /*derivs*/, T* /*type*/)
  {
    return (0.0);
  }

  /// Specific implementation for Point
  template<class VECTOR>
  inline double d_volume_type(const VECTOR& derivs, Geometry::Point* /*type*/)
  {
    double J[9];
    J[0]=derivs[0].x();
    J[3]=derivs[0].y();
    J[6]=derivs[0].z();
    J[1]=derivs[1].x();
    J[4]=derivs[1].y();
    J[7]=derivs[1].z();
    J[2]=derivs[2].x();
    J[5]=derivs[2].y();
    J[8]=derivs[2].z();

    return DetMatrix3x3(J);
  }

  /// calculate volume
  template <class VECTOR>
  inline double d_volume(const VECTOR& derivs)
  {
    return(d_volume_type(derivs,static_cast<typename VECTOR::value_type*>(0)));
  }

  template <class ElemBasis, class ElemData>
    double get_volume3(const ElemBasis *pEB, const ElemData &cd)
  {
    double volume=0.0;

    /// impelementation that is pure on stack
    StackVector<double,3> coords;
    for(int i=0; i<ElemBasis::GaussianNum; i++)
    {
      coords[0]=ElemBasis::GaussianPoints[i][0];
      coords[1]=ElemBasis::GaussianPoints[i][1];
      coords[2]=ElemBasis::GaussianPoints[i][2];

      StackVector<typename ElemBasis::value_type,3> derivs;
      pEB->derivate(coords, cd, derivs);
      volume+=ElemBasis::GaussianWeights[i]*d_volume(derivs);
    }
    return volume*pEB->volume();
  }

  /// area calculation on points
  template <class VECTOR1, class VECTOR2>
    inline double d_area_type(const VECTOR1& derivs, const VECTOR2& dv0, const VECTOR2& dv1, Geometry::Point* type)
  {
    const unsigned int dvsize = derivs.size();

    ENSURE_DIMENSIONS_MATCH(dv0.size(), dvsize, "Vector dv0 size not equal to derivs Vector size");
    ENSURE_DIMENSIONS_MATCH(dv1.size(), dvsize, "Vector dv0 size not equal to derivs Vector size");

    Geometry::Vector Jdv0(0,0,0), Jdv1(0,0,0);
    for(unsigned int i = 0; i<dvsize; i++) {
      Jdv0+=dv0[i]*Geometry::Vector(derivs[i].x(), derivs[i].y(), derivs[i].z());
      Jdv1+=dv1[i]*Geometry::Vector(derivs[i].x(), derivs[i].y(), derivs[i].z());
    }

    return Cross(Jdv0, Jdv1).length();
  }

  /// General template for any type of combination of containers
  template<class VECTOR1, class VECTOR2>
  inline double d_area(const VECTOR1& derivs, const VECTOR2& dv0, const VECTOR2& dv1)
  {
    return(d_area_type(derivs,dv0,dv1,static_cast<typename VECTOR1::value_type*>(0)));
  }



  template <class NumApprox, class ElemBasis, class ElemData>
    double get_area2(const ElemBasis *pEB, const unsigned face,
		     const ElemData &cd)
  {
    const double *v0 = pEB->unit_vertices[pEB->unit_faces[face][0]];
    const double *v1 = pEB->unit_vertices[pEB->unit_faces[face][1]];
    const double *v2 = pEB->unit_vertices[pEB->unit_faces[face][2]];

    StackVector<double,2> d0;
    StackVector<double,2> d1;

    d0[0]=v1[0]-v0[0];
    d0[1]=v1[1]-v0[1];
    d1[0]=v2[0]-v0[0];
    d1[1]=v2[1]-v0[1];
    double area=0.;

    StackVector<double,2> coords;
    for(int i=0; i<NumApprox::GaussianNum; i++) {
      coords[0]=v0[0]+NumApprox::GaussianPoints[i][0]*d0[0]+NumApprox::GaussianPoints[i][1]*d1[0];
      coords[1]=v0[1]+NumApprox::GaussianPoints[i][0]*d0[1]+NumApprox::GaussianPoints[i][1]*d1[1];

      StackVector<typename ElemBasis::value_type,2> derivs;
      pEB->derivate(coords, cd, derivs);
      area+=NumApprox::GaussianWeights[i]*d_area(derivs, d0, d1);
    }
    return (area*pEB->area(face));
  }



  template <class NumApprox, class ElemBasis, class ElemData>
    double get_area3(const ElemBasis *pEB, const unsigned face,
		     const ElemData &cd)
  {
    const double *v0 = pEB->unit_vertices[pEB->unit_faces[face][0]];
    const double *v1 = pEB->unit_vertices[pEB->unit_faces[face][1]];
    const double *v2 = pEB->unit_vertices[pEB->unit_faces[face][2]];

    StackVector<double,3> d0;
    StackVector<double,3> d1;
    d0[0]=v1[0]-v0[0];
    d0[1]=v1[1]-v0[1];
    d0[2]=v1[2]-v0[2];
    d1[0]=v2[0]-v0[0];
    d1[1]=v2[1]-v0[1];
    d1[2]=v2[2]-v0[2];
    double area=0.;

    StackVector<double,3> coords;
    for(int i=0; i<NumApprox::GaussianNum; i++) {
      coords[0]=v0[0]+NumApprox::GaussianPoints[i][0]*d0[0]+NumApprox::GaussianPoints[i][1]*d1[0];
      coords[1]=v0[1]+NumApprox::GaussianPoints[i][0]*d0[1]+NumApprox::GaussianPoints[i][1]*d1[1];
      coords[2]=v0[2]+NumApprox::GaussianPoints[i][0]*d0[2]+NumApprox::GaussianPoints[i][1]*d1[2];

      StackVector<typename ElemBasis::value_type,3> derivs;
      pEB->derivate(coords, cd, derivs);

      area+=NumApprox::GaussianWeights[i]*d_area(derivs, d0, d1);
    }
    return (area*pEB->area(face));
  }

  /// arc length calculation on points
   template <class VECTOR1, class VECTOR2>
  inline double d_arc_length(const VECTOR1& derivs, const VECTOR2& dv, Geometry::Point* type)
  {
    const unsigned int dvsize = dv.size();

    ENSURE_DIMENSIONS_MATCH(derivs.size(), dvsize, "Vector dv0 size not equal to derivs Vector size");

    double Jdv[3];
    Jdv[0] = Jdv[1] = Jdv[2] = 0.;
    for(unsigned int i = 0; i<dvsize; i++) {
      Jdv[0]+=dv[i]*derivs[i].x();
      Jdv[1]+=dv[i]*derivs[i].y();
      Jdv[2]+=dv[i]*derivs[i].z();
    }

    return sqrt(Jdv[0]*Jdv[0]+Jdv[1]*Jdv[1]+Jdv[2]*Jdv[2]);
  }


  template <class VECTOR1, class VECTOR2>
  inline double d_arc_length(const VECTOR1& derivs, const VECTOR2& dv)
  {
    return(d_arc_length_type(derivs,dv,static_cast<typename VECTOR1::value_type*>(0)));
  }


  template <class NumApprox, class ElemBasis, class ElemData>
    double get_arc1d_length(const ElemBasis *pEB, const unsigned edge,
			    const ElemData &cd)
  {
    const double *v0 = pEB->unit_vertices[pEB->unit_edges[edge][0]];
    const double *v1 = pEB->unit_vertices[pEB->unit_edges[edge][1]];
    StackVector<double,1> dv;
    dv[0]=v1[0]-v0[0];
    double arc_length=0.;

    StackVector<double,1> coords;
    for(int i=0; i<NumApprox::GaussianNum; i++) {
      coords[0]=v0[0]+NumApprox::GaussianPoints[i][0]*dv[0];
      StackVector<typename ElemBasis::value_type,1> derivs;
      pEB->derivate(coords, cd, derivs);

      arc_length+=NumApprox::GaussianWeights[i]*d_arc_length(derivs, dv);
    }
    return arc_length;
  }


  template <class NumApprox, class ElemBasis, class ElemData>
    double get_arc2d_length(const ElemBasis *pEB, const unsigned edge,
			    const ElemData &cd)
  {
    const double *v0 = pEB->unit_vertices[pEB->unit_edges[edge][0]];
    const double *v1 = pEB->unit_vertices[pEB->unit_edges[edge][1]];
    StackVector<double,2> dv;
    dv[0]=v1[0]-v0[0];
    dv[1]=v1[1]-v0[1];
    double arc_length=0.;

    StackVector<double,2> coords;
    for(int i=0; i<NumApprox::GaussianNum; i++) {
      coords[0]=v0[0]+NumApprox::GaussianPoints[i][0]*dv[0];
      coords[1]=v0[1]+NumApprox::GaussianPoints[i][0]*dv[1];
      StackVector<typename ElemBasis::value_type,2> derivs;
      pEB->derivate(coords, cd, derivs);
      arc_length+=NumApprox::GaussianWeights[i]*d_arc_length(derivs, dv);
    }
    return arc_length;
  }


  template <class NumApprox, class ElemBasis, class ElemData>
    double get_arc3d_length(const ElemBasis *pEB, const unsigned edge,
			    const ElemData &cd)
  {
    const double *v0 = pEB->unit_vertices[pEB->unit_edges[edge][0]];
    const double *v1 = pEB->unit_vertices[pEB->unit_edges[edge][1]];
    StackVector<double,3> dv;
    dv[0]=v1[0]-v0[0];
    dv[1]=v1[1]-v0[1];
    dv[2]=v1[2]-v0[2];
    double arc_length=0.;

    StackVector<double,3> coords;
    for(int i=0; i<NumApprox::GaussianNum; i++) {
      coords[0]=v0[0]+NumApprox::GaussianPoints[i][0]*dv[0];
      coords[1]=v0[1]+NumApprox::GaussianPoints[i][0]*dv[1];
      coords[2]=v0[2]+NumApprox::GaussianPoints[i][0]*dv[2];
      StackVector<typename ElemBasis::value_type,3> derivs;
      pEB->derivate(coords, cd, derivs);
      arc_length+=NumApprox::GaussianWeights[i]*d_arc_length(derivs, dv);
    }
    return arc_length;
  }


  //default case
  template <class T>
  inline T difference(const T& interp, const T& value)
  {
    return interp - value;
  }

  template <>
  inline Geometry::Point difference(const Geometry::Point& interp, const Geometry::Point& value)
  {
    return (interp - value).point();
  }


  template <class VECTOR1, class VECTOR2, class T>
  inline double getnextx1(VECTOR1 &x,
		     const T& y, const VECTOR2& yd)
  {
    double dx;
    if (yd[0])
    {
      dx = y/yd[0];
      x[0] -= dx;
    }
    else
      dx = 0;
    return sqrt(dx*dx);
  }

  template <class VECTOR1, class VECTOR2>
  inline double getnextx1(VECTOR1 &x,
		     const Geometry::Point& y, const VECTOR2& yd)
  {
    const Geometry::Point &yd0 = yd[0];

    const double dx = ((yd0.x() ? y.x()/yd0.x() : 0)+(yd0.y() ? y.y()/yd0.y() : 0)+(yd0.z() ? y.z()/yd0.z() : 0))/3.0;
    x[0] -= dx;

    return sqrt(dx*dx);
  }


  template <class VECTOR1, class VECTOR2, class T>
    double getnextx2(VECTOR1 &x,
		     const T& y, const VECTOR2& yd)
  {
    double dx, dy;
    if (yd[0]) {
      dx = y/yd[0];
      x[0] -= dx;
    }
    else
      dx = 0;

    if (yd[1]) {
      dy = y/yd[1];
      x[1] -= dy;
    }
    else
      dy = 0;

    return sqrt(dx*dx+dy*dy);
  }

  template <class VECTOR1, class VECTOR2>
    double getnextx2(VECTOR1 &x,
		     const Geometry::Point& y, const VECTOR2& yd)
  {
    const double J00=yd[0].x();
    const double J10=yd[0].y();
    const double J20=yd[0].z();
    const double J01=yd[1].x();
    const double J11=yd[1].y();
    const double J21=yd[1].z();
    const double detJ=-J10*J01+J10*J21-J00*J21-J11*J20+J11*J00+J01*J20;
    const double F1=y.x();
    const double F2=y.y();
    const double F3=y.z();

    if (detJ) {
      double dx=(F2*J01-F2*J21+J21*F1-J11*F1+F3*J11-J01*F3)/detJ;
      double dy=-(-J20*F2+J20*F1+J00*F2+F3*J10-F3*J00-F1*J10)/detJ;

      x[0] += dx;
      x[1] += dy;
      return  sqrt(dx*dx+dy*dy);
    }
    else
      return 0;
  }


  template <class VECTOR1, class VECTOR2, class T>
    double getnextx3(VECTOR1 &x,
		     const T& y, const VECTOR2& yd)
  {
    double dx, dy, dz;
    if (yd[0]) {
      dx = y/yd[0];
      x[0] -= dx;
    }
    else
      dx = 0;

    if (yd[1]) {
      dy = y/yd[1];
      x[1] -= dy;
    }
    else
      dy = 0;

    if (yd[2]) {
      dz = y/yd[2];
      x[2] -= dz;
    }
    else
      dz = 0;

    return sqrt(dx*dx+dy*dy+dz*dz);
  }


  template <class VECTOR1, class VECTOR2>
    double getnextx3(VECTOR1 &x,
		     const Geometry::Point& y, const VECTOR2& yd)
  {
    double J[9], JInv[9];

    J[0]=yd[0].x();
    J[3]=yd[0].y();
    J[6]=yd[0].z();
    J[1]=yd[1].x();
    J[4]=yd[1].y();
    J[7]=yd[1].z();
    J[2]=yd[2].x();
    J[5]=yd[2].y();
    J[8]=yd[2].z();

    InverseMatrix3x3(J, JInv);

    const double dx= JInv[0]*y.x()+JInv[1]*y.y()+JInv[2]*y.z();
    const double dy= JInv[3]*y.x()+JInv[4]*y.y()+JInv[5]*y.z();
    const double dz= JInv[6]*y.x()+JInv[7]*y.y()+JInv[8]*y.z();

    x[0] -= dx;
    x[1] -= dy;
    x[2] -= dz;

    return sqrt(dx*dx+dy*dy+dz*dz);
  }


  /// Class for searching of parametric coordinates related to a
  /// value in 3d meshes and fields
  /// More general function: find value in interpolation for given value
  /// Step 1: get a good guess on the domain, evaluate equally spaced points
  ///         on the domain and use the closest as our starting point for
  ///         Newton iteration. (implemented in derived class)
  /// Step 2: Newton iteration.
  ///         x_n+1 =x_n + y(x_n) * y'(x_n)^-1

  template <class ElemBasis>
  class Dim3Locate {
    public:
      typedef typename ElemBasis::value_type T;
      static const double thresholdDist; ///< Thresholds for coordinates checks
      static const double thresholdDist1; ///< 1+thresholdDist
      static const int maxsteps; ///< maximal steps for Newton search

      Dim3Locate() {}
      virtual ~Dim3Locate() {}

      /// find value in interpolation for given value
      template <class ElemData, class VECTOR>
      bool get_iterative(const ElemBasis *pEB, VECTOR &x,
			 const T& value, const ElemData &cd) const
      {
        StackVector<T,3> yd;
        for (int steps=0; steps<maxsteps; steps++)
        {
          T y = difference(pEB->interpolate(x, cd), value);
          pEB->derivate(x, cd, yd);
          double dist=getnextx3(x, y, yd);
          if (dist < thresholdDist)
            return true;
        }
        return false;
      }
  };

  template<class ElemBasis>
    const double Dim3Locate<ElemBasis>::thresholdDist=1e-7;
  template<class ElemBasis>
    const double Dim3Locate<ElemBasis>::thresholdDist1=1.+Dim3Locate::thresholdDist;
  template<class ElemBasis>
    const int Dim3Locate<ElemBasis>::maxsteps=100;

  /// Class for searching of parametric coordinates related to a
  /// value in 2d meshes and fields
  template <class ElemBasis>
  class Dim2Locate {
    public:
      typedef typename ElemBasis::value_type T;
      static const double thresholdDist; ///< Thresholds for coordinates checks
      static const double thresholdDist1; ///< 1+thresholdDist
      static const int maxsteps; ///< maximal steps for Newton search

      Dim2Locate() {}
      virtual ~Dim2Locate() {}

      /// find value in interpolation for given value
      template <class ElemData, class VECTOR>
        bool get_iterative(const ElemBasis *pEB, VECTOR &x,
         const T& value, const ElemData &cd) const
      {
        StackVector<T,2> yd;
        for (int steps=0; steps<maxsteps; steps++)
        {
          T y = difference(pEB->interpolate(x, cd), value);
          pEB->derivate(x, cd, yd);
          double dist=getnextx2(x, y, yd);
          if (dist < thresholdDist)
            return true;
        }
        return false;
      }
  };

  template<class ElemBasis>
    const double Dim2Locate<ElemBasis>::thresholdDist=1e-7;
  template<class ElemBasis>
    const double Dim2Locate<ElemBasis>::thresholdDist1=
    1.+Dim2Locate::thresholdDist;
  template<class ElemBasis>
    const int Dim2Locate<ElemBasis>::maxsteps=100;


  /// Class for searching of parametric coordinates related to a
  /// value in 1d meshes and fields
  template <class ElemBasis>
  class Dim1Locate {
    public:

      typedef typename ElemBasis::value_type T;
      static const double thresholdDist; ///< Thresholds for coordinates checks
      static const double thresholdDist1; ///< 1+thresholdDist
      static const int maxsteps; ///< maximal steps for Newton search

      Dim1Locate() {}
      virtual ~Dim1Locate() {}

      /// find value in interpolation for given value
      template <class ElemData, class VECTOR>
        bool get_iterative(const ElemBasis *pElem, VECTOR &x,
         const T& value, const ElemData &cd) const
      {
        StackVector<T,1> yd;

        for (int steps=0; steps<maxsteps; steps++)
        {
          T y = difference(pElem->interpolate(x, cd), value);
          pElem->derivate(x, cd, yd);
          double dist=getnextx1(x, y, yd);
          if (dist < thresholdDist)
            return true;
        }
        return false;
      }
    };

  template<class ElemBasis>
    const double Dim1Locate<ElemBasis>::thresholdDist=1e-7;
  template<class ElemBasis>
    const double Dim1Locate<ElemBasis>::thresholdDist1=1.+Dim1Locate::thresholdDist;
  template<class ElemBasis>
    const int Dim1Locate<ElemBasis>::maxsteps=100;




  // default case compiles for scalar types
  template <class T>
  inline bool compare_distance(const T &interp, const T &val,
			  double &cur_d, double dist)
  {
    double dv = interp - val;
    cur_d = sqrt(dv*dv);
    return  cur_d < dist;
  }

  inline bool compare_distance(const Geometry::Point &interp, const Geometry::Point &val,
			  double &cur_d, double dist)
  {
    Geometry::Vector v = interp - val;
    cur_d = v.length();
    return  cur_d < dist;
  }

  template <class VECTOR, class T>
  inline  bool check_zero_type(const VECTOR &val, T* /*type*/,double epsilon)
  {
    typename VECTOR::const_iterator iter = val.begin();
    while(iter != val.end())
    {
      const T &v=*iter++;
      if (fabs(v)>epsilon) return false;
    }
    return true;
  }

  template <class VECTOR>
  inline bool check_zero_type(const VECTOR &val, Geometry::Point* /*type*/, double epsilon)
  {
    typename VECTOR::const_iterator iter = val.begin();
    while(iter != val.end())
    {
      const Geometry::Point &v=*iter++;
      if (fabs(v.x())>epsilon || fabs(v.y())>epsilon || fabs(v.z())>epsilon)
        return false;
    }
    return true;
  }

  template <class VECTOR>
  inline double check_zero(const VECTOR& derivs,double epsilon = 1e-7)
  {
    return(check_zero_type(derivs,static_cast<typename VECTOR::value_type*>(0),epsilon));
  }

}}}

#endif

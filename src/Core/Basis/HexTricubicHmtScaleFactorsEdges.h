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
   Date:            November 5 2005
*/


#ifndef CORE_BASIS_HEXTRICUBICHMTSCALEFACTORSEDGES_H
#define CORE_BASIS_HEXTRICUBICHMTSCALEFACTORSEDGES_H 1

#include <Core/Persistent/PersistentSTL.h>
#include <Core/Basis/HexTrilinearLgn.h>

namespace SCIRun {
namespace Core {
namespace Basis {

/// Class for describing unit geometry of HexTricubicHmtScaleFactorsEdges
class HexTricubicHmtScaleFactorsEdgesUnitElement :
    public HexTrilinearLgnUnitElement {
public:
  HexTricubicHmtScaleFactorsEdgesUnitElement() {}
  virtual ~HexTricubicHmtScaleFactorsEdgesUnitElement() {}

  static int dofs() { return 64; } ///< return degrees of freedom
};


/// Class for handling of element of type hexahedron with
/// tricubic hermitian interpolation with scale factors
template <class T>
class HexTricubicHmtScaleFactorsEdges : public BasisAddDerivativesScaleFactorsEdges<T>,
                                   public HexApprox,
				   public HexGaussian3<double>,
           public HexSamplingSchemes,
				   public HexTricubicHmtScaleFactorsEdgesUnitElement,
           public HexElementWeights
{
public:
  typedef T value_type;

  HexTricubicHmtScaleFactorsEdges() {}
  virtual ~HexTricubicHmtScaleFactorsEdges() {}

  static int polynomial_order() { return 3; }


  template<class VECTOR>
  inline void get_weights(const VECTOR& coords, double *w) const
    { get_cubic_weights(coords,w); }

  template<class VECTOR>
  inline void get_derivate_weights(const VECTOR& coords, double *w) const
    { get_cubic_derivate_weights(coords,w); }

   /// get value at parametric coordinate
  template <class ElemData, class VECTOR>
  T interpolate(const VECTOR &coords, const ElemData &cd) const
  {
    double w[64];
    get_cubic_weights(coords, w);

    const double sx0=this->scalefactorse_[cd.edge0_index()];
    const double sx1=this->scalefactorse_[cd.edge0_index()];
    const double sx2=this->scalefactorse_[cd.edge2_index()];
    const double sx3=this->scalefactorse_[cd.edge2_index()];
    const double sx4=this->scalefactorse_[cd.edge4_index()];
    const double sx5=this->scalefactorse_[cd.edge4_index()];
    const double sx6=this->scalefactorse_[cd.edge6_index()];
    const double sx7=this->scalefactorse_[cd.edge6_index()];

    const double sy0=this->scalefactorse_[cd.edge3_index()];
    const double sy1=this->scalefactorse_[cd.edge1_index()];
    const double sy2=this->scalefactorse_[cd.edge1_index()];
    const double sy3=this->scalefactorse_[cd.edge3_index()];
    const double sy4=this->scalefactorse_[cd.edge7_index()];
    const double sy5=this->scalefactorse_[cd.edge5_index()];
    const double sy6=this->scalefactorse_[cd.edge5_index()];
    const double sy7=this->scalefactorse_[cd.edge7_index()];

    const double sz0=this->scalefactorse_[cd.edge8_index()];
    const double sz1=this->scalefactorse_[cd.edge9_index()];
    const double sz2=this->scalefactorse_[cd.edge10_index()];
    const double sz3=this->scalefactorse_[cd.edge11_index()];
    const double sz4=this->scalefactorse_[cd.edge8_index()];
    const double sz5=this->scalefactorse_[cd.edge9_index()];
    const double sz6=this->scalefactorse_[cd.edge10_index()];
    const double sz7=this->scalefactorse_[cd.edge11_index()];

    const T sdx0=this->derivs_[cd.node0_index()][0]*sx0;
    const T sdx1=this->derivs_[cd.node1_index()][0]*sx1;
    const T sdx2=this->derivs_[cd.node2_index()][0]*sx2;
    const T sdx3=this->derivs_[cd.node3_index()][0]*sx3;
    const T sdx4=this->derivs_[cd.node4_index()][0]*sx4;
    const T sdx5=this->derivs_[cd.node5_index()][0]*sx5;
    const T sdx6=this->derivs_[cd.node6_index()][0]*sx6;
    const T sdx7=this->derivs_[cd.node7_index()][0]*sx7;

    const T sdy0=this->derivs_[cd.node0_index()][1]*sy0;
    const T sdy1=this->derivs_[cd.node1_index()][1]*sy1;
    const T sdy2=this->derivs_[cd.node2_index()][1]*sy2;
    const T sdy3=this->derivs_[cd.node3_index()][1]*sy3;
    const T sdy4=this->derivs_[cd.node4_index()][1]*sy4;
    const T sdy5=this->derivs_[cd.node5_index()][1]*sy5;
    const T sdy6=this->derivs_[cd.node6_index()][1]*sy6;
    const T sdy7=this->derivs_[cd.node7_index()][1]*sy7;

    const T sdz0=this->derivs_[cd.node0_index()][2]*sz0;
    const T sdz1=this->derivs_[cd.node1_index()][2]*sz1;
    const T sdz2=this->derivs_[cd.node2_index()][2]*sz2;
    const T sdz3=this->derivs_[cd.node3_index()][2]*sz3;
    const T sdz4=this->derivs_[cd.node4_index()][2]*sz4;
    const T sdz5=this->derivs_[cd.node5_index()][2]*sz5;
    const T sdz6=this->derivs_[cd.node6_index()][2]*sz6;
    const T sdz7=this->derivs_[cd.node7_index()][2]*sz7;

    const T sdxy0=this->derivs_[cd.node0_index()][3]*sx0*sy0;
    const T sdxy1=this->derivs_[cd.node1_index()][3]*sx1*sy1;
    const T sdxy2=this->derivs_[cd.node2_index()][3]*sx2*sy2;
    const T sdxy3=this->derivs_[cd.node3_index()][3]*sx3*sy3;
    const T sdxy4=this->derivs_[cd.node4_index()][3]*sx4*sy4;
    const T sdxy5=this->derivs_[cd.node5_index()][3]*sx5*sy5;
    const T sdxy6=this->derivs_[cd.node6_index()][3]*sx6*sy6;
    const T sdxy7=this->derivs_[cd.node7_index()][3]*sx7*sy7;

    const T sdyz0=this->derivs_[cd.node0_index()][4]*sy0*sz0;
    const T sdyz1=this->derivs_[cd.node1_index()][4]*sy1*sz1;
    const T sdyz2=this->derivs_[cd.node2_index()][4]*sy2*sz2;
    const T sdyz3=this->derivs_[cd.node3_index()][4]*sy3*sz3;
    const T sdyz4=this->derivs_[cd.node4_index()][4]*sy4*sz4;
    const T sdyz5=this->derivs_[cd.node5_index()][4]*sy5*sz5;
    const T sdyz6=this->derivs_[cd.node6_index()][4]*sy6*sz6;
    const T sdyz7=this->derivs_[cd.node7_index()][4]*sy7*sz7;

    const T sdxz0=this->derivs_[cd.node0_index()][5]*sx0*sz0;
    const T sdxz1=this->derivs_[cd.node1_index()][5]*sx1*sz1;
    const T sdxz2=this->derivs_[cd.node2_index()][5]*sx2*sz2;
    const T sdxz3=this->derivs_[cd.node3_index()][5]*sx3*sz3;
    const T sdxz4=this->derivs_[cd.node4_index()][5]*sx4*sz4;
    const T sdxz5=this->derivs_[cd.node5_index()][5]*sx5*sz5;
    const T sdxz6=this->derivs_[cd.node6_index()][5]*sx6*sz6;
    const T sdxz7=this->derivs_[cd.node7_index()][5]*sx7*sz7;

    const T sdxyz0=this->derivs_[cd.node0_index()][6]*sx0*sy0*sz0;
    const T sdxyz1=this->derivs_[cd.node1_index()][6]*sx1*sy1*sz1;
    const T sdxyz2=this->derivs_[cd.node2_index()][6]*sx2*sy2*sz2;
    const T sdxyz3=this->derivs_[cd.node3_index()][6]*sx3*sy3*sz3;
    const T sdxyz4=this->derivs_[cd.node4_index()][6]*sx4*sy4*sz4;
    const T sdxyz5=this->derivs_[cd.node5_index()][6]*sx5*sy5*sz5;
    const T sdxyz6=this->derivs_[cd.node6_index()][6]*sx6*sy6*sz6;
    const T sdxyz7=this->derivs_[cd.node7_index()][6]*sx7*sy7*sz7;

    return (T)(
         w[0]  * cd.node0()+
	       w[1]  * sdx0	 +
	       w[2]  * sdy0	 +
	       w[3]  * sdz0	 +
	       w[4]  * sdxy0	 +
	       w[5]  * sdyz0	 +
	       w[6]  * sdxz0	 +
	       w[7]  * sdxyz0	 +
	       w[8]  * cd.node1()+
	       w[9]  * sdx1	 +
	       w[10] * sdy1	 +
	       w[11] * sdz1	 +
	       w[12] * sdxy1	 +
	       w[13] * sdyz1  	 +
	       w[14] * sdxz1	 +
	       w[15] * sdxyz1	 +
	       w[16] * cd.node2()+
	       w[17] * sdx2	 +
	       w[18] * sdy2	 +
	       w[19] * sdz2	 +
	       w[20] * sdxy2	 +
	       w[21] * sdyz2	 +
	       w[22] * sdxz2	 +
	       w[23] * sdxyz2 	 +
	       w[24] * cd.node3()+
	       w[25] * sdx3	 +
	       w[26] * sdy3	 +
	       w[27] * sdz3	 +
	       w[28] * sdxy3	 +
	       w[29] * sdyz3	 +
	       w[30] * sdxz3	 +
	       w[31] * sdxyz3	 +
	       w[32] * cd.node4()+
	       w[33] * sdx4	 +
	       w[34] * sdy4	 +
	       w[35] * sdz4	 +
	       w[36] * sdxy4	 +
	       w[37] * sdyz4	 +
	       w[38] * sdxz4	 +
	       w[39] * sdxyz4	 +
	       w[40] * cd.node5()+
	       w[41] * sdx5	 +
	       w[42] * sdy5	 +
	       w[43] * sdz5	 +
	       w[44] * sdxy5	 +
	       w[45] * sdyz5	 +
	       w[46] * sdxz5	 +
	       w[47] * sdxyz5	 +
	       w[48] * cd.node6()+
	       w[49] * sdx6	 +
	       w[50] * sdy6	 +
	       w[51] * sdz6	 +
	       w[52] * sdxy6	 +
	       w[53] * sdyz6	 +
	       w[54] * sdxz6	 +
	       w[55] * sdxyz6	 +
	       w[56] * cd.node7()+
	       w[57] * sdx7	 +
	       w[58] * sdy7	 +
	       w[59] * sdz7	 +
	       w[60] * sdxy7	 +
	       w[61] * sdyz7	 +
	       w[62] * sdxz7	 +
	       w[63] * sdxyz7);
  }


  /// get first derivative at parametric coordinate
  template <class ElemData, class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, const ElemData &cd,
		VECTOR2 &derivs) const
  {
    const double x=coords[0], y=coords[1], z=coords[2];
    const double x2=x*x;
    const double y2=y*y;
    const double z2=z*z;
    const double x12=(x-1)*(x-1);
    const double y12=(y-1)*(y-1);
    const double z12=(z-1)*(z-1);

    derivs.resize(3);

    const double sx0=this->scalefactorse_[cd.edge0_index()];
    const double sx1=this->scalefactorse_[cd.edge0_index()];
    const double sx2=this->scalefactorse_[cd.edge2_index()];
    const double sx3=this->scalefactorse_[cd.edge2_index()];
    const double sx4=this->scalefactorse_[cd.edge4_index()];
    const double sx5=this->scalefactorse_[cd.edge4_index()];
    const double sx6=this->scalefactorse_[cd.edge6_index()];
    const double sx7=this->scalefactorse_[cd.edge6_index()];

    const double sy0=this->scalefactorse_[cd.edge3_index()];
    const double sy1=this->scalefactorse_[cd.edge1_index()];
    const double sy2=this->scalefactorse_[cd.edge1_index()];
    const double sy3=this->scalefactorse_[cd.edge3_index()];
    const double sy4=this->scalefactorse_[cd.edge7_index()];
    const double sy5=this->scalefactorse_[cd.edge5_index()];
    const double sy6=this->scalefactorse_[cd.edge5_index()];
    const double sy7=this->scalefactorse_[cd.edge7_index()];

    const double sz0=this->scalefactorse_[cd.edge8_index()];
    const double sz1=this->scalefactorse_[cd.edge9_index()];
    const double sz2=this->scalefactorse_[cd.edge10_index()];
    const double sz3=this->scalefactorse_[cd.edge11_index()];
    const double sz4=this->scalefactorse_[cd.edge8_index()];
    const double sz5=this->scalefactorse_[cd.edge9_index()];
    const double sz6=this->scalefactorse_[cd.edge10_index()];
    const double sz7=this->scalefactorse_[cd.edge11_index()];

    const T sdx0=this->derivs_[cd.node0_index()][0]*sx0;
    const T sdx1=this->derivs_[cd.node1_index()][0]*sx1;
    const T sdx2=this->derivs_[cd.node2_index()][0]*sx2;
    const T sdx3=this->derivs_[cd.node3_index()][0]*sx3;
    const T sdx4=this->derivs_[cd.node4_index()][0]*sx4;
    const T sdx5=this->derivs_[cd.node5_index()][0]*sx5;
    const T sdx6=this->derivs_[cd.node6_index()][0]*sx6;
    const T sdx7=this->derivs_[cd.node7_index()][0]*sx7;

    const T sdy0=this->derivs_[cd.node0_index()][1]*sy0;
    const T sdy1=this->derivs_[cd.node1_index()][1]*sy1;
    const T sdy2=this->derivs_[cd.node2_index()][1]*sy2;
    const T sdy3=this->derivs_[cd.node3_index()][1]*sy3;
    const T sdy4=this->derivs_[cd.node4_index()][1]*sy4;
    const T sdy5=this->derivs_[cd.node5_index()][1]*sy5;
    const T sdy6=this->derivs_[cd.node6_index()][1]*sy6;
    const T sdy7=this->derivs_[cd.node7_index()][1]*sy7;

    const T sdz0=this->derivs_[cd.node0_index()][2]*sz0;
    const T sdz1=this->derivs_[cd.node1_index()][2]*sz1;
    const T sdz2=this->derivs_[cd.node2_index()][2]*sz2;
    const T sdz3=this->derivs_[cd.node3_index()][2]*sz3;
    const T sdz4=this->derivs_[cd.node4_index()][2]*sz4;
    const T sdz5=this->derivs_[cd.node5_index()][2]*sz5;
    const T sdz6=this->derivs_[cd.node6_index()][2]*sz6;
    const T sdz7=this->derivs_[cd.node7_index()][2]*sz7;

    const T sdxy0=this->derivs_[cd.node0_index()][3]*sx0*sy0;
    const T sdxy1=this->derivs_[cd.node1_index()][3]*sx1*sy1;
    const T sdxy2=this->derivs_[cd.node2_index()][3]*sx2*sy2;
    const T sdxy3=this->derivs_[cd.node3_index()][3]*sx3*sy3;
    const T sdxy4=this->derivs_[cd.node4_index()][3]*sx4*sy4;
    const T sdxy5=this->derivs_[cd.node5_index()][3]*sx5*sy5;
    const T sdxy6=this->derivs_[cd.node6_index()][3]*sx6*sy6;
    const T sdxy7=this->derivs_[cd.node7_index()][3]*sx7*sy7;

    const T sdyz0=this->derivs_[cd.node0_index()][4]*sy0*sz0;
    const T sdyz1=this->derivs_[cd.node1_index()][4]*sy1*sz1;
    const T sdyz2=this->derivs_[cd.node2_index()][4]*sy2*sz2;
    const T sdyz3=this->derivs_[cd.node3_index()][4]*sy3*sz3;
    const T sdyz4=this->derivs_[cd.node4_index()][4]*sy4*sz4;
    const T sdyz5=this->derivs_[cd.node5_index()][4]*sy5*sz5;
    const T sdyz6=this->derivs_[cd.node6_index()][4]*sy6*sz6;
    const T sdyz7=this->derivs_[cd.node7_index()][4]*sy7*sz7;

    const T sdxz0=this->derivs_[cd.node0_index()][5]*sx0*sz0;
    const T sdxz1=this->derivs_[cd.node1_index()][5]*sx1*sz1;
    const T sdxz2=this->derivs_[cd.node2_index()][5]*sx2*sz2;
    const T sdxz3=this->derivs_[cd.node3_index()][5]*sx3*sz3;
    const T sdxz4=this->derivs_[cd.node4_index()][5]*sx4*sz4;
    const T sdxz5=this->derivs_[cd.node5_index()][5]*sx5*sz5;
    const T sdxz6=this->derivs_[cd.node6_index()][5]*sx6*sz6;
    const T sdxz7=this->derivs_[cd.node7_index()][5]*sx7*sz7;

    const T sdxyz0=this->derivs_[cd.node0_index()][6]*sx0*sy0*sz0;
    const T sdxyz1=this->derivs_[cd.node1_index()][6]*sx1*sy1*sz1;
    const T sdxyz2=this->derivs_[cd.node2_index()][6]*sx2*sy2*sz2;
    const T sdxyz3=this->derivs_[cd.node3_index()][6]*sx3*sy3*sz3;
    const T sdxyz4=this->derivs_[cd.node4_index()][6]*sx4*sy4*sz4;
    const T sdxyz5=this->derivs_[cd.node5_index()][6]*sx5*sy5*sz5;
    const T sdxyz6=this->derivs_[cd.node6_index()][6]*sx6*sy6*sz6;
    const T sdxyz7=this->derivs_[cd.node7_index()][6]*sx7*sy7*sz7;

    derivs[0]=
      static_cast<typename VECTOR2::value_type>(6*(-1 + x)*x*y12*(1 + 2*y)*z12*(1 + 2*z)*cd.node0()
	+(1 - 4*x + 3*x2)*y12*(1 + 2*y)*z12*(1 + 2*z)*sdx0
	+6*(-1 + x)*x*y12*y*z12*(1 + 2*z)*sdy0
	+6*(-1 + x)*x*y12*(1 + 2*y)*z12*z*sdz0
	+(1 - 4*x + 3*x2)*y12*y*z12*(1 + 2*z)*sdxy0
	+6*(-1 + x)*x*y12*y*z12*z*sdyz0
	+(1 - 4*x + 3*x2)*y12*(1 + 2*y)*z12*z*sdxz0
	+(1 - 4*x + 3*x2)*y12*y*z12*z*sdxyz0
	-6*(-1 + x)*x*y12*(1 + 2*y)*z12*(1 + 2*z)*cd.node1()
	+x*(-2 + 3*x)*y12*(1 + 2*y)*z12*(1 + 2*z)*sdx1
	-6*(-1 + x)*x*y12*y*z12*(1 + 2*z)*sdy1
	-6*(-1 + x)*x*y12*(1 + 2*y)*z12*z*sdz1
	+x*(-2 + 3*x)*y12*y*z12*(1 + 2*z)*sdxy1
	-6*(-1 + x)*x*y12*y*z12*z*sdyz1
	+x*(-2 + 3*x)*y12*(1 + 2*y)*z12*z*sdxz1
	+x*(-2 + 3*x)*y12*y*z12*z*sdxyz1
	+6*(-1 + x)*x*y2*(-3 + 2*y)*z12*(1 + 2*z)*cd.node2()
	-(x*(-2 + 3*x)*y2*(-3 + 2*y)*z12*(1 + 2*z))*sdx2
	-6*(-1 + x)*x*(-1 + y)*y2*z12*(1 + 2*z)*sdy2
	+6*(-1 + x)*x*y2*(-3 + 2*y)*z12*z*sdz2
	+x*(-2 + 3*x)*(-1 + y)*y2*z12*(1 + 2*z)*sdxy2
	-6*(-1 + x)*x*(-1 + y)*y2*z12*z*sdyz2
	-(x*(-2 + 3*x)*y2*(-3 + 2*y)*z12*z)*sdxz2
	+x*(-2 + 3*x)*(-1 + y)*y2*z12*z*sdxyz2
	-6*(-1 + x)*x*y2*(-3 + 2*y)*z12*(1 + 2*z)*cd.node3()
	-((1 - 4*x + 3*x2)*y2*(-3 + 2*y)*z12*(1 + 2*z))*sdx3
	+6*(-1 + x)*x*(-1 + y)*y2*z12*(1 + 2*z)*sdy3
	-6*(-1 + x)*x*y2*(-3 + 2*y)*z12*z*sdz3
	+(1 - 4*x + 3*x2)*(-1 + y)*y2*z12*(1 + 2*z)*sdxy3
	+6*(-1 + x)*x*(-1 + y)*y2*z12*z*sdyz3
	-((1 - 4*x + 3*x2)*y2*(-3 + 2*y)*z12*z)*sdxz3
	+(1 - 4*x + 3*x2)*(-1 + y)*y2*z12*z*sdxyz3
	-6*(-1 + x)*x*y12*(1 + 2*y)*z2*(-3 + 2*z)*cd.node4()
	-((1 - 4*x + 3*x2)*y12*(1 + 2*y)*z2*(-3 + 2*z))*sdx4
	-6*(-1 + x)*x*y12*y*z2*(-3 + 2*z)*sdy4
	+6*(-1 + x)*x*y12*(1 + 2*y)*(-1 + z)*z2*sdz4
	-((1 - 4*x + 3*x2)*y12*y*z2*(-3 + 2*z))*sdxy4
	+6*(-1 + x)*x*y12*y*(-1 + z)*z2*sdyz4
	+(1 - 4*x + 3*x2)*y12*(1 + 2*y)*(-1 + z)*z2*sdxz4
	+(1 - 4*x + 3*x2)*y12*y*(-1 + z)*z2*sdxyz4
	+6*(-1 + x)*x*y12*(1 + 2*y)*z2*(-3 + 2*z)*cd.node5()
	-(x*(-2 + 3*x)*y12*(1 + 2*y)*z2*(-3 + 2*z))*sdx5
	+6*(-1 + x)*x*y12*y*z2*(-3 + 2*z)*sdy5
	-6*(-1 + x)*x*y12*(1 + 2*y)*(-1 + z)*z2*sdz5
	-(x*(-2 + 3*x)*y12*y*z2*(-3 + 2*z))*sdxy5
	-6*(-1 + x)*x*y12*y*(-1 + z)*z2*sdyz5
	+x*(-2 + 3*x)*y12*(1 + 2*y)*(-1 + z)*z2*sdxz5
	+x*(-2 + 3*x)*y12*y*(-1 + z)*z2*sdxyz5
	-6*(-1 + x)*x*y2*(-3 + 2*y)*z2*(-3 + 2*z)*cd.node6()
	+x*(-2 + 3*x)*y2*(-3 + 2*y)*z2*(-3 + 2*z)*sdx6
	+6*(-1 + x)*x*(-1 + y)*y2*z2*(-3 + 2*z)*sdy6
	+6*(-1 + x)*x*y2*(-3 + 2*y)*(-1 + z)*z2*sdz6
	-(x*(-2 + 3*x)*(-1 + y)*y2*z2*(-3 + 2*z))*sdxy6
	-6*(-1 + x)*x*(-1 + y)*y2*(-1 + z)*z2*sdyz6
	-(x*(-2 + 3*x)*y2*(-3 + 2*y)*(-1 + z)*z2)*sdxz6
	+x*(-2 + 3*x)*(-1 + y)*y2*(-1 + z)*z2*sdxyz6
	+6*(-1 + x)*x*y2*(-3 +  2*y)*z2*(-3 + 2*z)*cd.node7()
	+(1 - 4*x + 3*x2)*y2*(-3 + 2*y)*z2*(-3 + 2*z)*sdx7
	-6*(-1 + x)*x*(-1 + y)*y2*z2*(-3 + 2*z)*sdy7
	-6*(-1 + x)*x*y2*(-3 + 2*y)*(-1 + z)*z2*sdz7
	-((1 - 4*x + 3*x2)*(-1 + y)*y2*z2*(-3 + 2*z))*sdxy7
	+6*(-1 + x)*x*(-1 + y)*y2*(-1 + z)*z2*sdyz7
	-((1 - 4*x + 3*x2)*y2*(-3 + 2*y)*(-1 + z)*z2)*sdxz7
	+(1 - 4*x + 3*x2)*(-1 + y)*y2*(-1 + z)*z2*sdxyz7);

    derivs[1]=
      static_cast<typename VECTOR2::value_type>(6*x12*(1 + 2*x)*(-1 + y)*y*z12*(1 + 2*z)*cd.node0()
	+6*x12*x*(-1 + y)*y*z12*(1 + 2*z)*sdx0
	+x12*(1 + 2*x)*(1 - 4*y + 3*y2)*z12*(1 + 2*z)*sdy0
	+6*x12*(1 + 2*x)*(-1 + y)*y*z12*z*sdz0
	+x12*x*(1 - 4*y + 3*y2)*z12*(1 + 2*z)*sdxy0
	+x12*(1 + 2*x)*(1 - 4*y + 3*y2)*z12*z*sdyz0
	+6*x12*x*(-1 + y)*y*z12*z*sdxz0
	+x12*x*(1 - 4*y + 3*y2)*z12*z*sdxyz0
	-6*x2*(-3 + 2*x)*(-1 + y)*y*z12*(1 + 2*z)*cd.node1()
	+6*(-1 + x)*x2*(-1 + y)*y*z12*(1 + 2*z)*sdx1
	-(x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*z12*(1 + 2*z))*sdy1
	-6*x2*(-3 + 2*x)*(-1 + y)*y*z12*z*sdz1
	+(-1 + x)*x2*(1 - 4*y + 3*y2)*z12*(1 + 2*z)*sdxy1
	-(x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*z12*z)*sdyz1
	+6*(-1 + x)*x2*(-1 + y)*y*z12*z*sdxz1
	+(-1 + x)*x2*(1 - 4*y + 3*y2)*z12*z*sdxyz1
	+6*x2*(-3 + 2*x)*(-1 + y)*y*z12*(1 + 2*z)*cd.node2()
	-6*(-1 + x)*x2*(-1 + y)*y*z12*(1 + 2*z)*sdx2
	-(x2*(-3 + 2*x)*y*(-2 + 3*y)*z12*(1 + 2*z))*sdy2
	+6*x2*(-3 + 2*x)*(-1 + y)*y*z12*z*sdz2
	+(-1 + x)*x2*y*(-2 + 3*y)*z12*(1 + 2*z)*sdxy2
	-(x2*(-3 + 2*x)*y*(-2 + 3*y)*z12*z)*sdyz2
	-6*(-1 + x)*x2*(-1 + y)*y*z12*z*sdxz2
	+(-1 + x)*x2*y*(-2 + 3*y)*z12*z*sdxyz2
	-6*x12*(1 + 2*x)*(-1 + y)*y*z12*(1 + 2*z)*cd.node3()
	-6*x12*x*(-1 + y)*y*z12*(1 + 2*z)*sdx3
	+x12*(1 + 2*x)*y*(-2 + 3*y)*z12*(1 + 2*z)*sdy3
	-6*x12*(1 + 2*x)*(-1 + y)*y*z12*z*sdz3
	+x12*x*y*(-2 + 3*y)*z12*(1 + 2*z)*sdxy3
	+x12*(1 + 2*x)*y*(-2 + 3*y)*z12*z*sdyz3
	-6*x12*x*(-1 + y)*y*z12*z*sdxz3
	+x12*x*y*(-2 + 3*y)*z12*z*sdxyz3
	-6*x12*(1 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z)*cd.node4()
	-6*x12*x*(-1 + y)*y*z2*(-3 + 2*z)*sdx4
	-(x12*(1 + 2*x)*(1 - 4*y + 3*y2)*z2*(-3 + 2*z))*sdy4
	+6*x12*(1 + 2*x)*(-1 + y)*y*(-1 + z)*z2*sdz4
	-(x12*x*(1 - 4*y + 3*y2)*z2*(-3 + 2*z))*sdxy4
	+x12*(1 + 2*x)*(1 - 4*y + 3*y2)*(-1 + z)*z2*sdyz4
	+6*x12*x*(-1 + y)*y*(-1 + z)*z2*sdxz4
	+x12*x*(1 - 4*y + 3*y2)*(-1 + z)*z2*sdxyz4
	+6*x2*(-3 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z)*cd.node5()
	-6*(-1 + x)*x2*(-1 + y)*y*z2*(-3 + 2*z)*sdx5
	+x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*z2*(-3 + 2*z)*sdy5
	-6*x2*(-3 + 2*x)*(-1 + y)*y*(-1 + z)*z2*sdz5
	-((-1 + x)*x2*(1 - 4*y + 3*y2)*z2*(-3 + 2*z))*sdxy5
	-(x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*(-1 + z)*z2)*sdyz5
	+6*(-1 + x)*x2*(-1 + y)*y*(-1 + z)*z2*sdxz5
	+(-1 + x)*x2*(1 - 4*y + 3*y2)*(-1 + z)*z2*sdxyz5
	-6*x2*(-3 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z)*cd.node6()
	+6*(-1 + x)*x2*(-1 + y)*y*z2*(-3 + 2*z)*sdx6
	+x2*(-3 + 2*x)*y*(-2 + 3*y)*z2*(-3 + 2*z)*sdy6
	+6*x2*(-3 + 2*x)*(-1 + y)*y*(-1 + z)*z2*sdz6
	-((-1 + x)*x2*y*(-2 + 3*y)*z2*(-3 + 2*z))*sdxy6
	-(x2*(-3 + 2*x)*y*(-2 + 3*y)*(-1 + z)*z2)*sdyz6
	-6*(-1 + x)*x2*(-1 + y)*y*(-1 + z)*z2*sdxz6
	+(-1 + x)*x2*y*(-2 + 3*y)*(-1 + z)*z2*sdxyz6
	+6*x12*(1 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z)*cd.node7()
	+6*x12*x*(-1 + y)*y*z2*(-3 + 2*z)*sdx7
	-(x12*(1 + 2*x)*y*(-2 + 3*y)*z2*(-3 + 2*z))*sdy7
	-6*x12*(1 + 2*x)*(-1 + y)*y*(-1 + z)*z2*sdz7
	-(x12*x*y*(-2 + 3*y)*z2*(-3 + 2*z))*sdxy7
	+x12*(1 + 2*x)*y*(-2 + 3*y)*(-1 + z)*z2*sdyz7
	-6*x12*x*(-1 + y)*y*(-1 + z)*z2*sdxz7
	+x12*x*y*(-2 + 3*y)*(-1 + z)*z2*sdxyz7);

    derivs[2]=
      static_cast<typename VECTOR2::value_type>(6*x12*(1 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z*cd.node0()
	+6*x12*x*y12*(1 + 2*y)*(-1 + z)*z*sdx0
	+6*x12*(1 + 2*x)*y12*y*(-1 + z)*z*sdy0
	+x12*(1 + 2*x)*y12*(1 + 2*y)*(1 - 4*z + 3*z2)*sdz0
	+6*x12*x*y12*y*(-1 + z)*z*sdxy0
	+x12*(1 + 2*x)*y12*y*(1 - 4*z + 3*z2)*sdyz0
	+x12*x*y12*(1 + 2*y)*(1 - 4*z + 3*z2)*sdxz0
	+x12*x*y12*y*(1 - 4*z + 3*z2)*sdxyz0
	-6*x2*(-3 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z*cd.node1()
	+6*(-1 + x)*x2*y12*(1 + 2*y)*(-1 + z)*z*sdx1
	-6*x2*(-3 + 2*x)*y12*y*(-1 + z)*z*sdy1
	-(x2*(-3 + 2*x)*y12*(1 + 2*y)*(1 - 4*z + 3*z2))*sdz1
	+6*(-1 + x)*x2*y12*y*(-1 + z)*z*sdxy1
	-(x2*(-3 + 2*x)*y12*y*(1 - 4*z + 3*z2))*sdyz1
	+(-1 + x)*x2*y12*(1 + 2*y)*(1 - 4*z + 3*z2)*sdxz1
	+(-1 + x)*x2*y12*y*(1 - 4*z + 3*z2)*sdxyz1
	+6*x2*(-3 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z*cd.node2()
	-6*(-1 + x)*x2*y2*(-3 + 2*y)*(-1 + z)*z*sdx2
	-6*x2*(-3 + 2*x)*(-1 + y)*y2*(-1 + z)*z*sdy2
	+x2*(-3 + 2*x)*y2*(-3 + 2*y)*(1 - 4*z + 3*z2)*sdz2
	+6*(-1 + x)*x2*(-1 + y)*y2*(-1 + z)*z*sdxy2
	-(x2*(-3 + 2*x)*(-1 + y)*y2*(1 - 4*z + 3*z2))*sdyz2
	-((-1 + x)*x2*y2*(-3 + 2*y)*(1 - 4*z + 3*z2))*sdxz2
	+(-1 + x)*x2*(-1 + y)*y2*(1 - 4*z + 3*z2)*sdxyz2
	-6*x12*(1 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z*cd.node3()
	-6*x12*x*y2*(-3 + 2*y)*(-1 + z)*z*sdx3
	+6*x12*(1 + 2*x)*(-1 + y)*y2*(-1 + z)*z*sdy3
	-(x12*(1 + 2*x)*y2*(-3 + 2*y)*(1 - 4*z + 3*z2))*sdz3
	+6*x12*x*(-1 + y)*y2*(-1 + z)*z*sdxy3
	+x12*(1 + 2*x)*(-1 + y)*y2*(1 - 4*z + 3*z2)*sdyz3
	-(x12*x*y2*(-3 + 2*y)*(1 - 4*z + 3*z2))*sdxz3
	+x12*x*(-1 + y)*y2*(1 - 4*z + 3*z2)*sdxyz3
	-6*x12*(1 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z*cd.node4()
	-6*x12*x*y12*(1 + 2*y)*(-1 + z)*z*sdx4
	-6*x12*(1 + 2*x)*y12*y*(-1 + z)*z*sdy4
	+x12*(1 + 2*x)*y12*(1 + 2*y)*z*(-2 + 3*z)*sdz4
	-6*x12*x*y12*y*(-1 + z)*z*sdxy4
	+x12*(1 + 2*x)*y12*y*z*(-2 + 3*z)*sdyz4
	+x12*x*y12*(1 + 2*y)*z*(-2 + 3*z)*sdxz4
	+x12*x*y12*y*z*(-2 + 3*z)*sdxyz4
	+6*x2*(-3 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z*cd.node5()
	-6*(-1 + x)*x2*y12*(1 + 2*y)*(-1 + z)*z*sdx5
	+6*x2*(-3 + 2*x)*y12*y*(-1 + z)*z*sdy5
	-(x2*(-3 + 2*x)*y12*(1 + 2*y)*z*(-2 + 3*z))*sdz5
	-6*(-1 + x)*x2*y12*y*(-1 + z)*z*sdxy5
	-(x2*(-3 + 2*x)*y12*y*z*(-2 + 3*z))*sdyz5
	+(-1 + x)*x2*y12*(1 + 2*y)*z*(-2 + 3*z)*sdxz5
	+(-1 + x)*x2*y12*y*z*(-2 + 3*z)*sdxyz5
	-6*x2*(-3 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z*cd.node6()
	+6*(-1 + x)*x2*y2*(-3 + 2*y)*(-1 + z)*z*sdx6
	+6*x2*(-3 + 2*x)*(-1 + y)*y2*(-1 + z)*z*sdy6
	+x2*(-3 + 2*x)*y2*(-3 + 2*y)*z*(-2 + 3*z)*sdz6
	-6*(-1 + x)*x2*(-1 + y)*y2*(-1 + z)*z*sdxy6
	-(x2*(-3 + 2*x)*(-1 + y)*y2*z*(-2 + 3*z))*sdyz6
	-((-1 + x)*x2*y2*(-3 + 2*y)*z*(-2 + 3*z))*sdxz6
	+(-1 + x)*x2*(-1 + y)*y2*z*(-2 + 3*z)*sdxyz6
	+6*x12*(1 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z*cd.node7()
	+6*x12*x*y2*(-3 + 2*y)*(-1 + z)*z*sdx7
	-6*x12*(1 + 2*x)*(-1 + y)*y2*(-1 + z)*z*sdy7
	-(x12*(1 + 2*x)*y2*(-3 + 2*y)*z*(-2 + 3*z))*sdz7
	-6*x12*x*(-1 + y)*y2*(-1 + z)*z*sdxy7
	+x12*(1 + 2*x)*(-1 + y)*y2*z*(-2 + 3*z)*sdyz7
	-(x12*x*y2*(-3 + 2*y)*z*(-2 + 3*z))*sdxz7
	+x12*x*(-1 + y)*y2*z*(-2 + 3*z)*sdxyz7);
  }

  /// get parametric coordinate for value within the element
  template <class ElemData, class VECTOR>
  bool get_coords(VECTOR &coords, const T& value,
		  const ElemData &cd) const
  {
    HexLocate< HexTricubicHmtScaleFactorsEdges<T> > CL;
    return CL.get_coords(this, coords, value, cd);
  }

  /// get arc length for edge
  template <class ElemData>
  double get_arc_length(const unsigned edge, const ElemData &cd) const
  {
    return get_arc3d_length<CrvGaussian2<double> >(this, edge, cd);
  }

  /// get area
  template <class ElemData>
    double get_area(const unsigned face, const ElemData &cd) const
  {
    return get_area3<QuadGaussian3<double> >(this, face, cd);
  }

  /// get volume
  template <class ElemData>
    double get_volume(const ElemData & cd) const
  {
    return get_volume3(this, cd);
  }
  static const std::string type_name(int n = -1);

  virtual void io (Piostream& str);

};

template <class T>
const std::string
HexTricubicHmtScaleFactorsEdges<T>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("HexTricubicHmtScaleFactorsEdges");
    return nm;
  } else {
    return find_type_name((T *)0);
  }
}






}}
template <class T>
const TypeDescription*
  get_type_description(Core::Basis::HexTricubicHmtScaleFactorsEdges<T> *)
{
  static TypeDescription* td = 0;
  if(!td){
    const TypeDescription *sub = get_type_description((T*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("HexTricubicHmtScaleFactorsEdges",
      subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::BASIS_E);
  }
  return td;
}

const int HEXTRICUBICHMTSCALEFACTORSEDGES_VERSION = 1;
template <class T>
void
  Core::Basis::HexTricubicHmtScaleFactorsEdges<T>::io(Piostream &stream)
{
  stream.begin_class(get_type_description(this)->get_name(),
    HEXTRICUBICHMTSCALEFACTORSEDGES_VERSION);
  Pio(stream, this->derivs_);
  Pio(stream, this->scalefactorse_);
  stream.end_class();
}
}

#endif

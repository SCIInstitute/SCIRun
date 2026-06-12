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
*/

#ifndef CORE_BASIS_HEXTRICUBICHMTSCALEFACTORSCOMMON_H
#define CORE_BASIS_HEXTRICUBICHMTSCALEFACTORSCOMMON_H 1

// Private implementation header: included only by HexTricubicHmtScaleFactors.h
// and HexTricubicHmtScaleFactorsEdges.h.
//
// Both classes perform identical scaled-Hermite evaluation once the per-node
// scale factors sx[8]/sy[8]/sz[8] have been assembled.  The two helpers below
// hold the shared body so neither class has to duplicate it.

namespace SCIRun {
namespace Core {
namespace Basis {

/// Evaluate the 64-term scaled tricubic Hermite weighted sum for interpolate().
///
/// @param w      64-entry weight array from get_cubic_weights()
/// @param cd     element-data object providing node values and node indices
/// @param coeff  basis derivative coefficients (this->derivs_)
/// @param sx     per-node x scale factors (size 8)
/// @param sy     per-node y scale factors (size 8)
/// @param sz     per-node z scale factors (size 8)
template <class T, class ElemData, class DerivsCoeff>
inline T hexScaledBasisInterpolate(
    const double* w,
    const ElemData& cd,
    const DerivsCoeff& coeff,
    const double (&sx)[8], const double (&sy)[8], const double (&sz)[8])
{
  const T sdx0=coeff[cd.node0_index()][0]*sx[0];
  const T sdx1=coeff[cd.node1_index()][0]*sx[1];
  const T sdx2=coeff[cd.node2_index()][0]*sx[2];
  const T sdx3=coeff[cd.node3_index()][0]*sx[3];
  const T sdx4=coeff[cd.node4_index()][0]*sx[4];
  const T sdx5=coeff[cd.node5_index()][0]*sx[5];
  const T sdx6=coeff[cd.node6_index()][0]*sx[6];
  const T sdx7=coeff[cd.node7_index()][0]*sx[7];

  const T sdy0=coeff[cd.node0_index()][1]*sy[0];
  const T sdy1=coeff[cd.node1_index()][1]*sy[1];
  const T sdy2=coeff[cd.node2_index()][1]*sy[2];
  const T sdy3=coeff[cd.node3_index()][1]*sy[3];
  const T sdy4=coeff[cd.node4_index()][1]*sy[4];
  const T sdy5=coeff[cd.node5_index()][1]*sy[5];
  const T sdy6=coeff[cd.node6_index()][1]*sy[6];
  const T sdy7=coeff[cd.node7_index()][1]*sy[7];

  const T sdz0=coeff[cd.node0_index()][2]*sz[0];
  const T sdz1=coeff[cd.node1_index()][2]*sz[1];
  const T sdz2=coeff[cd.node2_index()][2]*sz[2];
  const T sdz3=coeff[cd.node3_index()][2]*sz[3];
  const T sdz4=coeff[cd.node4_index()][2]*sz[4];
  const T sdz5=coeff[cd.node5_index()][2]*sz[5];
  const T sdz6=coeff[cd.node6_index()][2]*sz[6];
  const T sdz7=coeff[cd.node7_index()][2]*sz[7];

  const T sdxy0=coeff[cd.node0_index()][3]*sx[0]*sy[0];
  const T sdxy1=coeff[cd.node1_index()][3]*sx[1]*sy[1];
  const T sdxy2=coeff[cd.node2_index()][3]*sx[2]*sy[2];
  const T sdxy3=coeff[cd.node3_index()][3]*sx[3]*sy[3];
  const T sdxy4=coeff[cd.node4_index()][3]*sx[4]*sy[4];
  const T sdxy5=coeff[cd.node5_index()][3]*sx[5]*sy[5];
  const T sdxy6=coeff[cd.node6_index()][3]*sx[6]*sy[6];
  const T sdxy7=coeff[cd.node7_index()][3]*sx[7]*sy[7];

  const T sdyz0=coeff[cd.node0_index()][4]*sy[0]*sz[0];
  const T sdyz1=coeff[cd.node1_index()][4]*sy[1]*sz[1];
  const T sdyz2=coeff[cd.node2_index()][4]*sy[2]*sz[2];
  const T sdyz3=coeff[cd.node3_index()][4]*sy[3]*sz[3];
  const T sdyz4=coeff[cd.node4_index()][4]*sy[4]*sz[4];
  const T sdyz5=coeff[cd.node5_index()][4]*sy[5]*sz[5];
  const T sdyz6=coeff[cd.node6_index()][4]*sy[6]*sz[6];
  const T sdyz7=coeff[cd.node7_index()][4]*sy[7]*sz[7];

  const T sdxz0=coeff[cd.node0_index()][5]*sx[0]*sz[0];
  const T sdxz1=coeff[cd.node1_index()][5]*sx[1]*sz[1];
  const T sdxz2=coeff[cd.node2_index()][5]*sx[2]*sz[2];
  const T sdxz3=coeff[cd.node3_index()][5]*sx[3]*sz[3];
  const T sdxz4=coeff[cd.node4_index()][5]*sx[4]*sz[4];
  const T sdxz5=coeff[cd.node5_index()][5]*sx[5]*sz[5];
  const T sdxz6=coeff[cd.node6_index()][5]*sx[6]*sz[6];
  const T sdxz7=coeff[cd.node7_index()][5]*sx[7]*sz[7];

  const T sdxyz0=coeff[cd.node0_index()][6]*sx[0]*sy[0]*sz[0];
  const T sdxyz1=coeff[cd.node1_index()][6]*sx[1]*sy[1]*sz[1];
  const T sdxyz2=coeff[cd.node2_index()][6]*sx[2]*sy[2]*sz[2];
  const T sdxyz3=coeff[cd.node3_index()][6]*sx[3]*sy[3]*sz[3];
  const T sdxyz4=coeff[cd.node4_index()][6]*sx[4]*sy[4]*sz[4];
  const T sdxyz5=coeff[cd.node5_index()][6]*sx[5]*sy[5]*sz[5];
  const T sdxyz6=coeff[cd.node6_index()][6]*sx[6]*sy[6]*sz[6];
  const T sdxyz7=coeff[cd.node7_index()][6]*sx[7]*sy[7]*sz[7];

  return (T)(
       w[0]  * cd.node0()+
       w[1]  * sdx0   +
       w[2]  * sdy0   +
       w[3]  * sdz0   +
       w[4]  * sdxy0  +
       w[5]  * sdyz0  +
       w[6]  * sdxz0  +
       w[7]  * sdxyz0 +
       w[8]  * cd.node1()+
       w[9]  * sdx1   +
       w[10] * sdy1   +
       w[11] * sdz1   +
       w[12] * sdxy1  +
       w[13] * sdyz1  +
       w[14] * sdxz1  +
       w[15] * sdxyz1 +
       w[16] * cd.node2()+
       w[17] * sdx2   +
       w[18] * sdy2   +
       w[19] * sdz2   +
       w[20] * sdxy2  +
       w[21] * sdyz2  +
       w[22] * sdxz2  +
       w[23] * sdxyz2 +
       w[24] * cd.node3()+
       w[25] * sdx3   +
       w[26] * sdy3   +
       w[27] * sdz3   +
       w[28] * sdxy3  +
       w[29] * sdyz3  +
       w[30] * sdxz3  +
       w[31] * sdxyz3 +
       w[32] * cd.node4()+
       w[33] * sdx4   +
       w[34] * sdy4   +
       w[35] * sdz4   +
       w[36] * sdxy4  +
       w[37] * sdyz4  +
       w[38] * sdxz4  +
       w[39] * sdxyz4 +
       w[40] * cd.node5()+
       w[41] * sdx5   +
       w[42] * sdy5   +
       w[43] * sdz5   +
       w[44] * sdxy5  +
       w[45] * sdyz5  +
       w[46] * sdxz5  +
       w[47] * sdxyz5 +
       w[48] * cd.node6()+
       w[49] * sdx6   +
       w[50] * sdy6   +
       w[51] * sdz6   +
       w[52] * sdxy6  +
       w[53] * sdyz6  +
       w[54] * sdxz6  +
       w[55] * sdxyz6 +
       w[56] * cd.node7()+
       w[57] * sdx7   +
       w[58] * sdy7   +
       w[59] * sdz7   +
       w[60] * sdxy7  +
       w[61] * sdyz7  +
       w[62] * sdxz7  +
       w[63] * sdxyz7);
}


/// Compute the three partial derivatives for derivate().
///
/// @param x,y,z       parametric coordinates
/// @param x2,y2,z2    squares of parametric coordinates
/// @param x12,y12,z12 squares of (coord - 1)
/// @param cd          element-data object
/// @param coeff       basis derivative coefficients (this->derivs_)
/// @param sx,sy,sz    per-node scale factors (size 8 each)
/// @param result      output vector; must already be resized to 3 by the caller
template <class T, class ElemData, class DerivsCoeff, class VECTOR2>
inline void hexScaledBasisDerivate(
    double x, double y, double z,
    double x2, double y2, double z2,
    double x12, double y12, double z12,
    const ElemData& cd,
    const DerivsCoeff& coeff,
    const double (&sx)[8], const double (&sy)[8], const double (&sz)[8],
    VECTOR2& result)
{
  const T sdx0=coeff[cd.node0_index()][0]*sx[0];
  const T sdx1=coeff[cd.node1_index()][0]*sx[1];
  const T sdx2=coeff[cd.node2_index()][0]*sx[2];
  const T sdx3=coeff[cd.node3_index()][0]*sx[3];
  const T sdx4=coeff[cd.node4_index()][0]*sx[4];
  const T sdx5=coeff[cd.node5_index()][0]*sx[5];
  const T sdx6=coeff[cd.node6_index()][0]*sx[6];
  const T sdx7=coeff[cd.node7_index()][0]*sx[7];

  const T sdy0=coeff[cd.node0_index()][1]*sy[0];
  const T sdy1=coeff[cd.node1_index()][1]*sy[1];
  const T sdy2=coeff[cd.node2_index()][1]*sy[2];
  const T sdy3=coeff[cd.node3_index()][1]*sy[3];
  const T sdy4=coeff[cd.node4_index()][1]*sy[4];
  const T sdy5=coeff[cd.node5_index()][1]*sy[5];
  const T sdy6=coeff[cd.node6_index()][1]*sy[6];
  const T sdy7=coeff[cd.node7_index()][1]*sy[7];

  const T sdz0=coeff[cd.node0_index()][2]*sz[0];
  const T sdz1=coeff[cd.node1_index()][2]*sz[1];
  const T sdz2=coeff[cd.node2_index()][2]*sz[2];
  const T sdz3=coeff[cd.node3_index()][2]*sz[3];
  const T sdz4=coeff[cd.node4_index()][2]*sz[4];
  const T sdz5=coeff[cd.node5_index()][2]*sz[5];
  const T sdz6=coeff[cd.node6_index()][2]*sz[6];
  const T sdz7=coeff[cd.node7_index()][2]*sz[7];

  const T sdxy0=coeff[cd.node0_index()][3]*sx[0]*sy[0];
  const T sdxy1=coeff[cd.node1_index()][3]*sx[1]*sy[1];
  const T sdxy2=coeff[cd.node2_index()][3]*sx[2]*sy[2];
  const T sdxy3=coeff[cd.node3_index()][3]*sx[3]*sy[3];
  const T sdxy4=coeff[cd.node4_index()][3]*sx[4]*sy[4];
  const T sdxy5=coeff[cd.node5_index()][3]*sx[5]*sy[5];
  const T sdxy6=coeff[cd.node6_index()][3]*sx[6]*sy[6];
  const T sdxy7=coeff[cd.node7_index()][3]*sx[7]*sy[7];

  const T sdyz0=coeff[cd.node0_index()][4]*sy[0]*sz[0];
  const T sdyz1=coeff[cd.node1_index()][4]*sy[1]*sz[1];
  const T sdyz2=coeff[cd.node2_index()][4]*sy[2]*sz[2];
  const T sdyz3=coeff[cd.node3_index()][4]*sy[3]*sz[3];
  const T sdyz4=coeff[cd.node4_index()][4]*sy[4]*sz[4];
  const T sdyz5=coeff[cd.node5_index()][4]*sy[5]*sz[5];
  const T sdyz6=coeff[cd.node6_index()][4]*sy[6]*sz[6];
  const T sdyz7=coeff[cd.node7_index()][4]*sy[7]*sz[7];

  const T sdxz0=coeff[cd.node0_index()][5]*sx[0]*sz[0];
  const T sdxz1=coeff[cd.node1_index()][5]*sx[1]*sz[1];
  const T sdxz2=coeff[cd.node2_index()][5]*sx[2]*sz[2];
  const T sdxz3=coeff[cd.node3_index()][5]*sx[3]*sz[3];
  const T sdxz4=coeff[cd.node4_index()][5]*sx[4]*sz[4];
  const T sdxz5=coeff[cd.node5_index()][5]*sx[5]*sz[5];
  const T sdxz6=coeff[cd.node6_index()][5]*sx[6]*sz[6];
  const T sdxz7=coeff[cd.node7_index()][5]*sx[7]*sz[7];

  const T sdxyz0=coeff[cd.node0_index()][6]*sx[0]*sy[0]*sz[0];
  const T sdxyz1=coeff[cd.node1_index()][6]*sx[1]*sy[1]*sz[1];
  const T sdxyz2=coeff[cd.node2_index()][6]*sx[2]*sy[2]*sz[2];
  const T sdxyz3=coeff[cd.node3_index()][6]*sx[3]*sy[3]*sz[3];
  const T sdxyz4=coeff[cd.node4_index()][6]*sx[4]*sy[4]*sz[4];
  const T sdxyz5=coeff[cd.node5_index()][6]*sx[5]*sy[5]*sz[5];
  const T sdxyz6=coeff[cd.node6_index()][6]*sx[6]*sy[6]*sz[6];
  const T sdxyz7=coeff[cd.node7_index()][6]*sx[7]*sy[7]*sz[7];

  result[0]=
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

  result[1]=
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

  result[2]=
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

}}} // namespace SCIRun::Core::Basis

#endif // CORE_BASIS_HEXTRICUBICHMTSCALEFACTORSCOMMON_H

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
// scale factors sx[8]/sy[8]/sz[8] have been assembled.  The helpers below
// hold the shared body so neither class duplicates it.

namespace SCIRun {
namespace Core {
namespace Basis {

/// Per-node scaled derivative coefficients used by both interpolate() and
/// derivate().  Computed once from basis coefficients and scale factors.
template <class T>
struct HexScaledBasisCoeffs {
  T dx[8], dy[8], dz[8], dxy[8], dyz[8], dxz[8], dxyz[8];
};

/// Compute the 7 × 8 = 56 scaled derivative coefficients.
/// coeff[nodeIdx][k] are the stored Hermite derivative coefficients;
/// sx/sy/sz are the per-node scale factors assembled by the caller.
template <class T, class ElemData, class DerivsCoeff>
inline HexScaledBasisCoeffs<T> computeHexScaledBasisCoeffs(
    const ElemData& cd,
    const DerivsCoeff& coeff,
    const double (&sx)[8], const double (&sy)[8], const double (&sz)[8])
{
  HexScaledBasisCoeffs<T> sd;

  sd.dx[0]=coeff[cd.node0_index()][0]*sx[0];
  sd.dx[1]=coeff[cd.node1_index()][0]*sx[1];
  sd.dx[2]=coeff[cd.node2_index()][0]*sx[2];
  sd.dx[3]=coeff[cd.node3_index()][0]*sx[3];
  sd.dx[4]=coeff[cd.node4_index()][0]*sx[4];
  sd.dx[5]=coeff[cd.node5_index()][0]*sx[5];
  sd.dx[6]=coeff[cd.node6_index()][0]*sx[6];
  sd.dx[7]=coeff[cd.node7_index()][0]*sx[7];

  sd.dy[0]=coeff[cd.node0_index()][1]*sy[0];
  sd.dy[1]=coeff[cd.node1_index()][1]*sy[1];
  sd.dy[2]=coeff[cd.node2_index()][1]*sy[2];
  sd.dy[3]=coeff[cd.node3_index()][1]*sy[3];
  sd.dy[4]=coeff[cd.node4_index()][1]*sy[4];
  sd.dy[5]=coeff[cd.node5_index()][1]*sy[5];
  sd.dy[6]=coeff[cd.node6_index()][1]*sy[6];
  sd.dy[7]=coeff[cd.node7_index()][1]*sy[7];

  sd.dz[0]=coeff[cd.node0_index()][2]*sz[0];
  sd.dz[1]=coeff[cd.node1_index()][2]*sz[1];
  sd.dz[2]=coeff[cd.node2_index()][2]*sz[2];
  sd.dz[3]=coeff[cd.node3_index()][2]*sz[3];
  sd.dz[4]=coeff[cd.node4_index()][2]*sz[4];
  sd.dz[5]=coeff[cd.node5_index()][2]*sz[5];
  sd.dz[6]=coeff[cd.node6_index()][2]*sz[6];
  sd.dz[7]=coeff[cd.node7_index()][2]*sz[7];

  sd.dxy[0]=coeff[cd.node0_index()][3]*sx[0]*sy[0];
  sd.dxy[1]=coeff[cd.node1_index()][3]*sx[1]*sy[1];
  sd.dxy[2]=coeff[cd.node2_index()][3]*sx[2]*sy[2];
  sd.dxy[3]=coeff[cd.node3_index()][3]*sx[3]*sy[3];
  sd.dxy[4]=coeff[cd.node4_index()][3]*sx[4]*sy[4];
  sd.dxy[5]=coeff[cd.node5_index()][3]*sx[5]*sy[5];
  sd.dxy[6]=coeff[cd.node6_index()][3]*sx[6]*sy[6];
  sd.dxy[7]=coeff[cd.node7_index()][3]*sx[7]*sy[7];

  sd.dyz[0]=coeff[cd.node0_index()][4]*sy[0]*sz[0];
  sd.dyz[1]=coeff[cd.node1_index()][4]*sy[1]*sz[1];
  sd.dyz[2]=coeff[cd.node2_index()][4]*sy[2]*sz[2];
  sd.dyz[3]=coeff[cd.node3_index()][4]*sy[3]*sz[3];
  sd.dyz[4]=coeff[cd.node4_index()][4]*sy[4]*sz[4];
  sd.dyz[5]=coeff[cd.node5_index()][4]*sy[5]*sz[5];
  sd.dyz[6]=coeff[cd.node6_index()][4]*sy[6]*sz[6];
  sd.dyz[7]=coeff[cd.node7_index()][4]*sy[7]*sz[7];

  sd.dxz[0]=coeff[cd.node0_index()][5]*sx[0]*sz[0];
  sd.dxz[1]=coeff[cd.node1_index()][5]*sx[1]*sz[1];
  sd.dxz[2]=coeff[cd.node2_index()][5]*sx[2]*sz[2];
  sd.dxz[3]=coeff[cd.node3_index()][5]*sx[3]*sz[3];
  sd.dxz[4]=coeff[cd.node4_index()][5]*sx[4]*sz[4];
  sd.dxz[5]=coeff[cd.node5_index()][5]*sx[5]*sz[5];
  sd.dxz[6]=coeff[cd.node6_index()][5]*sx[6]*sz[6];
  sd.dxz[7]=coeff[cd.node7_index()][5]*sx[7]*sz[7];

  sd.dxyz[0]=coeff[cd.node0_index()][6]*sx[0]*sy[0]*sz[0];
  sd.dxyz[1]=coeff[cd.node1_index()][6]*sx[1]*sy[1]*sz[1];
  sd.dxyz[2]=coeff[cd.node2_index()][6]*sx[2]*sy[2]*sz[2];
  sd.dxyz[3]=coeff[cd.node3_index()][6]*sx[3]*sy[3]*sz[3];
  sd.dxyz[4]=coeff[cd.node4_index()][6]*sx[4]*sy[4]*sz[4];
  sd.dxyz[5]=coeff[cd.node5_index()][6]*sx[5]*sy[5]*sz[5];
  sd.dxyz[6]=coeff[cd.node6_index()][6]*sx[6]*sy[6]*sz[6];
  sd.dxyz[7]=coeff[cd.node7_index()][6]*sx[7]*sy[7]*sz[7];

  return sd;
}


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
  const HexScaledBasisCoeffs<T> sd =
      computeHexScaledBasisCoeffs<T>(cd, coeff, sx, sy, sz);

  return (T)(
       w[0]  * cd.node0()  +
       w[1]  * sd.dx[0]   +
       w[2]  * sd.dy[0]   +
       w[3]  * sd.dz[0]   +
       w[4]  * sd.dxy[0]  +
       w[5]  * sd.dyz[0]  +
       w[6]  * sd.dxz[0]  +
       w[7]  * sd.dxyz[0] +
       w[8]  * cd.node1()  +
       w[9]  * sd.dx[1]   +
       w[10] * sd.dy[1]   +
       w[11] * sd.dz[1]   +
       w[12] * sd.dxy[1]  +
       w[13] * sd.dyz[1]  +
       w[14] * sd.dxz[1]  +
       w[15] * sd.dxyz[1] +
       w[16] * cd.node2()  +
       w[17] * sd.dx[2]   +
       w[18] * sd.dy[2]   +
       w[19] * sd.dz[2]   +
       w[20] * sd.dxy[2]  +
       w[21] * sd.dyz[2]  +
       w[22] * sd.dxz[2]  +
       w[23] * sd.dxyz[2] +
       w[24] * cd.node3()  +
       w[25] * sd.dx[3]   +
       w[26] * sd.dy[3]   +
       w[27] * sd.dz[3]   +
       w[28] * sd.dxy[3]  +
       w[29] * sd.dyz[3]  +
       w[30] * sd.dxz[3]  +
       w[31] * sd.dxyz[3] +
       w[32] * cd.node4()  +
       w[33] * sd.dx[4]   +
       w[34] * sd.dy[4]   +
       w[35] * sd.dz[4]   +
       w[36] * sd.dxy[4]  +
       w[37] * sd.dyz[4]  +
       w[38] * sd.dxz[4]  +
       w[39] * sd.dxyz[4] +
       w[40] * cd.node5()  +
       w[41] * sd.dx[5]   +
       w[42] * sd.dy[5]   +
       w[43] * sd.dz[5]   +
       w[44] * sd.dxy[5]  +
       w[45] * sd.dyz[5]  +
       w[46] * sd.dxz[5]  +
       w[47] * sd.dxyz[5] +
       w[48] * cd.node6()  +
       w[49] * sd.dx[6]   +
       w[50] * sd.dy[6]   +
       w[51] * sd.dz[6]   +
       w[52] * sd.dxy[6]  +
       w[53] * sd.dyz[6]  +
       w[54] * sd.dxz[6]  +
       w[55] * sd.dxyz[6] +
       w[56] * cd.node7()  +
       w[57] * sd.dx[7]   +
       w[58] * sd.dy[7]   +
       w[59] * sd.dz[7]   +
       w[60] * sd.dxy[7]  +
       w[61] * sd.dyz[7]  +
       w[62] * sd.dxz[7]  +
       w[63] * sd.dxyz[7]);
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
  const HexScaledBasisCoeffs<T> sd =
      computeHexScaledBasisCoeffs<T>(cd, coeff, sx, sy, sz);

  result[0]=
    static_cast<typename VECTOR2::value_type>(6*(-1 + x)*x*y12*(1 + 2*y)*z12*(1 + 2*z)*cd.node0()
	+(1 - 4*x + 3*x2)*y12*(1 + 2*y)*z12*(1 + 2*z)*sd.dx[0]
	+6*(-1 + x)*x*y12*y*z12*(1 + 2*z)*sd.dy[0]
	+6*(-1 + x)*x*y12*(1 + 2*y)*z12*z*sd.dz[0]
	+(1 - 4*x + 3*x2)*y12*y*z12*(1 + 2*z)*sd.dxy[0]
	+6*(-1 + x)*x*y12*y*z12*z*sd.dyz[0]
	+(1 - 4*x + 3*x2)*y12*(1 + 2*y)*z12*z*sd.dxz[0]
	+(1 - 4*x + 3*x2)*y12*y*z12*z*sd.dxyz[0]
	-6*(-1 + x)*x*y12*(1 + 2*y)*z12*(1 + 2*z)*cd.node1()
	+x*(-2 + 3*x)*y12*(1 + 2*y)*z12*(1 + 2*z)*sd.dx[1]
	-6*(-1 + x)*x*y12*y*z12*(1 + 2*z)*sd.dy[1]
	-6*(-1 + x)*x*y12*(1 + 2*y)*z12*z*sd.dz[1]
	+x*(-2 + 3*x)*y12*y*z12*(1 + 2*z)*sd.dxy[1]
	-6*(-1 + x)*x*y12*y*z12*z*sd.dyz[1]
	+x*(-2 + 3*x)*y12*(1 + 2*y)*z12*z*sd.dxz[1]
	+x*(-2 + 3*x)*y12*y*z12*z*sd.dxyz[1]
	+6*(-1 + x)*x*y2*(-3 + 2*y)*z12*(1 + 2*z)*cd.node2()
	-(x*(-2 + 3*x)*y2*(-3 + 2*y)*z12*(1 + 2*z))*sd.dx[2]
	-6*(-1 + x)*x*(-1 + y)*y2*z12*(1 + 2*z)*sd.dy[2]
	+6*(-1 + x)*x*y2*(-3 + 2*y)*z12*z*sd.dz[2]
	+x*(-2 + 3*x)*(-1 + y)*y2*z12*(1 + 2*z)*sd.dxy[2]
	-6*(-1 + x)*x*(-1 + y)*y2*z12*z*sd.dyz[2]
	-(x*(-2 + 3*x)*y2*(-3 + 2*y)*z12*z)*sd.dxz[2]
	+x*(-2 + 3*x)*(-1 + y)*y2*z12*z*sd.dxyz[2]
	-6*(-1 + x)*x*y2*(-3 + 2*y)*z12*(1 + 2*z)*cd.node3()
	-((1 - 4*x + 3*x2)*y2*(-3 + 2*y)*z12*(1 + 2*z))*sd.dx[3]
	+6*(-1 + x)*x*(-1 + y)*y2*z12*(1 + 2*z)*sd.dy[3]
	-6*(-1 + x)*x*y2*(-3 + 2*y)*z12*z*sd.dz[3]
	+(1 - 4*x + 3*x2)*(-1 + y)*y2*z12*(1 + 2*z)*sd.dxy[3]
	+6*(-1 + x)*x*(-1 + y)*y2*z12*z*sd.dyz[3]
	-((1 - 4*x + 3*x2)*y2*(-3 + 2*y)*z12*z)*sd.dxz[3]
	+(1 - 4*x + 3*x2)*(-1 + y)*y2*z12*z*sd.dxyz[3]
	-6*(-1 + x)*x*y12*(1 + 2*y)*z2*(-3 + 2*z)*cd.node4()
	-((1 - 4*x + 3*x2)*y12*(1 + 2*y)*z2*(-3 + 2*z))*sd.dx[4]
	-6*(-1 + x)*x*y12*y*z2*(-3 + 2*z)*sd.dy[4]
	+6*(-1 + x)*x*y12*(1 + 2*y)*(-1 + z)*z2*sd.dz[4]
	-((1 - 4*x + 3*x2)*y12*y*z2*(-3 + 2*z))*sd.dxy[4]
	+6*(-1 + x)*x*y12*y*(-1 + z)*z2*sd.dyz[4]
	+(1 - 4*x + 3*x2)*y12*(1 + 2*y)*(-1 + z)*z2*sd.dxz[4]
	+(1 - 4*x + 3*x2)*y12*y*(-1 + z)*z2*sd.dxyz[4]
	+6*(-1 + x)*x*y12*(1 + 2*y)*z2*(-3 + 2*z)*cd.node5()
	-(x*(-2 + 3*x)*y12*(1 + 2*y)*z2*(-3 + 2*z))*sd.dx[5]
	+6*(-1 + x)*x*y12*y*z2*(-3 + 2*z)*sd.dy[5]
	-6*(-1 + x)*x*y12*(1 + 2*y)*(-1 + z)*z2*sd.dz[5]
	-(x*(-2 + 3*x)*y12*y*z2*(-3 + 2*z))*sd.dxy[5]
	-6*(-1 + x)*x*y12*y*(-1 + z)*z2*sd.dyz[5]
	+x*(-2 + 3*x)*y12*(1 + 2*y)*(-1 + z)*z2*sd.dxz[5]
	+x*(-2 + 3*x)*y12*y*(-1 + z)*z2*sd.dxyz[5]
	-6*(-1 + x)*x*y2*(-3 + 2*y)*z2*(-3 + 2*z)*cd.node6()
	+x*(-2 + 3*x)*y2*(-3 + 2*y)*z2*(-3 + 2*z)*sd.dx[6]
	+6*(-1 + x)*x*(-1 + y)*y2*z2*(-3 + 2*z)*sd.dy[6]
	+6*(-1 + x)*x*y2*(-3 + 2*y)*(-1 + z)*z2*sd.dz[6]
	-(x*(-2 + 3*x)*(-1 + y)*y2*z2*(-3 + 2*z))*sd.dxy[6]
	-6*(-1 + x)*x*(-1 + y)*y2*(-1 + z)*z2*sd.dyz[6]
	-(x*(-2 + 3*x)*y2*(-3 + 2*y)*(-1 + z)*z2)*sd.dxz[6]
	+x*(-2 + 3*x)*(-1 + y)*y2*(-1 + z)*z2*sd.dxyz[6]
	+6*(-1 + x)*x*y2*(-3 +  2*y)*z2*(-3 + 2*z)*cd.node7()
	+(1 - 4*x + 3*x2)*y2*(-3 + 2*y)*z2*(-3 + 2*z)*sd.dx[7]
	-6*(-1 + x)*x*(-1 + y)*y2*z2*(-3 + 2*z)*sd.dy[7]
	-6*(-1 + x)*x*y2*(-3 + 2*y)*(-1 + z)*z2*sd.dz[7]
	-((1 - 4*x + 3*x2)*(-1 + y)*y2*z2*(-3 + 2*z))*sd.dxy[7]
	+6*(-1 + x)*x*(-1 + y)*y2*(-1 + z)*z2*sd.dyz[7]
	-((1 - 4*x + 3*x2)*y2*(-3 + 2*y)*(-1 + z)*z2)*sd.dxz[7]
	+(1 - 4*x + 3*x2)*(-1 + y)*y2*(-1 + z)*z2*sd.dxyz[7]);

  result[1]=
    static_cast<typename VECTOR2::value_type>(6*x12*(1 + 2*x)*(-1 + y)*y*z12*(1 + 2*z)*cd.node0()
	+6*x12*x*(-1 + y)*y*z12*(1 + 2*z)*sd.dx[0]
	+x12*(1 + 2*x)*(1 - 4*y + 3*y2)*z12*(1 + 2*z)*sd.dy[0]
	+6*x12*(1 + 2*x)*(-1 + y)*y*z12*z*sd.dz[0]
	+x12*x*(1 - 4*y + 3*y2)*z12*(1 + 2*z)*sd.dxy[0]
	+x12*(1 + 2*x)*(1 - 4*y + 3*y2)*z12*z*sd.dyz[0]
	+6*x12*x*(-1 + y)*y*z12*z*sd.dxz[0]
	+x12*x*(1 - 4*y + 3*y2)*z12*z*sd.dxyz[0]
	-6*x2*(-3 + 2*x)*(-1 + y)*y*z12*(1 + 2*z)*cd.node1()
	+6*(-1 + x)*x2*(-1 + y)*y*z12*(1 + 2*z)*sd.dx[1]
	-(x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*z12*(1 + 2*z))*sd.dy[1]
	-6*x2*(-3 + 2*x)*(-1 + y)*y*z12*z*sd.dz[1]
	+(-1 + x)*x2*(1 - 4*y + 3*y2)*z12*(1 + 2*z)*sd.dxy[1]
	-(x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*z12*z)*sd.dyz[1]
	+6*(-1 + x)*x2*(-1 + y)*y*z12*z*sd.dxz[1]
	+(-1 + x)*x2*(1 - 4*y + 3*y2)*z12*z*sd.dxyz[1]
	+6*x2*(-3 + 2*x)*(-1 + y)*y*z12*(1 + 2*z)*cd.node2()
	-6*(-1 + x)*x2*(-1 + y)*y*z12*(1 + 2*z)*sd.dx[2]
	-(x2*(-3 + 2*x)*y*(-2 + 3*y)*z12*(1 + 2*z))*sd.dy[2]
	+6*x2*(-3 + 2*x)*(-1 + y)*y*z12*z*sd.dz[2]
	+(-1 + x)*x2*y*(-2 + 3*y)*z12*(1 + 2*z)*sd.dxy[2]
	-(x2*(-3 + 2*x)*y*(-2 + 3*y)*z12*z)*sd.dyz[2]
	-6*(-1 + x)*x2*(-1 + y)*y*z12*z*sd.dxz[2]
	+(-1 + x)*x2*y*(-2 + 3*y)*z12*z*sd.dxyz[2]
	-6*x12*(1 + 2*x)*(-1 + y)*y*z12*(1 + 2*z)*cd.node3()
	-6*x12*x*(-1 + y)*y*z12*(1 + 2*z)*sd.dx[3]
	+x12*(1 + 2*x)*y*(-2 + 3*y)*z12*(1 + 2*z)*sd.dy[3]
	-6*x12*(1 + 2*x)*(-1 + y)*y*z12*z*sd.dz[3]
	+x12*x*y*(-2 + 3*y)*z12*(1 + 2*z)*sd.dxy[3]
	+x12*(1 + 2*x)*y*(-2 + 3*y)*z12*z*sd.dyz[3]
	-6*x12*x*(-1 + y)*y*z12*z*sd.dxz[3]
	+x12*x*y*(-2 + 3*y)*z12*z*sd.dxyz[3]
	-6*x12*(1 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z)*cd.node4()
	-6*x12*x*(-1 + y)*y*z2*(-3 + 2*z)*sd.dx[4]
	-(x12*(1 + 2*x)*(1 - 4*y + 3*y2)*z2*(-3 + 2*z))*sd.dy[4]
	+6*x12*(1 + 2*x)*(-1 + y)*y*(-1 + z)*z2*sd.dz[4]
	-(x12*x*(1 - 4*y + 3*y2)*z2*(-3 + 2*z))*sd.dxy[4]
	+x12*(1 + 2*x)*(1 - 4*y + 3*y2)*(-1 + z)*z2*sd.dyz[4]
	+6*x12*x*(-1 + y)*y*(-1 + z)*z2*sd.dxz[4]
	+x12*x*(1 - 4*y + 3*y2)*(-1 + z)*z2*sd.dxyz[4]
	+6*x2*(-3 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z)*cd.node5()
	-6*(-1 + x)*x2*(-1 + y)*y*z2*(-3 + 2*z)*sd.dx[5]
	+x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*z2*(-3 + 2*z)*sd.dy[5]
	-6*x2*(-3 + 2*x)*(-1 + y)*y*(-1 + z)*z2*sd.dz[5]
	-((-1 + x)*x2*(1 - 4*y + 3*y2)*z2*(-3 + 2*z))*sd.dxy[5]
	-(x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*(-1 + z)*z2)*sd.dyz[5]
	+6*(-1 + x)*x2*(-1 + y)*y*(-1 + z)*z2*sd.dxz[5]
	+(-1 + x)*x2*(1 - 4*y + 3*y2)*(-1 + z)*z2*sd.dxyz[5]
	-6*x2*(-3 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z)*cd.node6()
	+6*(-1 + x)*x2*(-1 + y)*y*z2*(-3 + 2*z)*sd.dx[6]
	+x2*(-3 + 2*x)*y*(-2 + 3*y)*z2*(-3 + 2*z)*sd.dy[6]
	+6*x2*(-3 + 2*x)*(-1 + y)*y*(-1 + z)*z2*sd.dz[6]
	-((-1 + x)*x2*y*(-2 + 3*y)*z2*(-3 + 2*z))*sd.dxy[6]
	-(x2*(-3 + 2*x)*y*(-2 + 3*y)*(-1 + z)*z2)*sd.dyz[6]
	-6*(-1 + x)*x2*(-1 + y)*y*(-1 + z)*z2*sd.dxz[6]
	+(-1 + x)*x2*y*(-2 + 3*y)*(-1 + z)*z2*sd.dxyz[6]
	+6*x12*(1 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z)*cd.node7()
	+6*x12*x*(-1 + y)*y*z2*(-3 + 2*z)*sd.dx[7]
	-(x12*(1 + 2*x)*y*(-2 + 3*y)*z2*(-3 + 2*z))*sd.dy[7]
	-6*x12*(1 + 2*x)*(-1 + y)*y*(-1 + z)*z2*sd.dz[7]
	-(x12*x*y*(-2 + 3*y)*z2*(-3 + 2*z))*sd.dxy[7]
	+x12*(1 + 2*x)*y*(-2 + 3*y)*(-1 + z)*z2*sd.dyz[7]
	-6*x12*x*(-1 + y)*y*(-1 + z)*z2*sd.dxz[7]
	+x12*x*y*(-2 + 3*y)*(-1 + z)*z2*sd.dxyz[7]);

  result[2]=
    static_cast<typename VECTOR2::value_type>(6*x12*(1 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z*cd.node0()
	+6*x12*x*y12*(1 + 2*y)*(-1 + z)*z*sd.dx[0]
	+6*x12*(1 + 2*x)*y12*y*(-1 + z)*z*sd.dy[0]
	+x12*(1 + 2*x)*y12*(1 + 2*y)*(1 - 4*z + 3*z2)*sd.dz[0]
	+6*x12*x*y12*y*(-1 + z)*z*sd.dxy[0]
	+x12*(1 + 2*x)*y12*y*(1 - 4*z + 3*z2)*sd.dyz[0]
	+x12*x*y12*(1 + 2*y)*(1 - 4*z + 3*z2)*sd.dxz[0]
	+x12*x*y12*y*(1 - 4*z + 3*z2)*sd.dxyz[0]
	-6*x2*(-3 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z*cd.node1()
	+6*(-1 + x)*x2*y12*(1 + 2*y)*(-1 + z)*z*sd.dx[1]
	-6*x2*(-3 + 2*x)*y12*y*(-1 + z)*z*sd.dy[1]
	-(x2*(-3 + 2*x)*y12*(1 + 2*y)*(1 - 4*z + 3*z2))*sd.dz[1]
	+6*(-1 + x)*x2*y12*y*(-1 + z)*z*sd.dxy[1]
	-(x2*(-3 + 2*x)*y12*y*(1 - 4*z + 3*z2))*sd.dyz[1]
	+(-1 + x)*x2*y12*(1 + 2*y)*(1 - 4*z + 3*z2)*sd.dxz[1]
	+(-1 + x)*x2*y12*y*(1 - 4*z + 3*z2)*sd.dxyz[1]
	+6*x2*(-3 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z*cd.node2()
	-6*(-1 + x)*x2*y2*(-3 + 2*y)*(-1 + z)*z*sd.dx[2]
	-6*x2*(-3 + 2*x)*(-1 + y)*y2*(-1 + z)*z*sd.dy[2]
	+x2*(-3 + 2*x)*y2*(-3 + 2*y)*(1 - 4*z + 3*z2)*sd.dz[2]
	+6*(-1 + x)*x2*(-1 + y)*y2*(-1 + z)*z*sd.dxy[2]
	-(x2*(-3 + 2*x)*(-1 + y)*y2*(1 - 4*z + 3*z2))*sd.dyz[2]
	-((-1 + x)*x2*y2*(-3 + 2*y)*(1 - 4*z + 3*z2))*sd.dxz[2]
	+(-1 + x)*x2*(-1 + y)*y2*(1 - 4*z + 3*z2)*sd.dxyz[2]
	-6*x12*(1 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z*cd.node3()
	-6*x12*x*y2*(-3 + 2*y)*(-1 + z)*z*sd.dx[3]
	+6*x12*(1 + 2*x)*(-1 + y)*y2*(-1 + z)*z*sd.dy[3]
	-(x12*(1 + 2*x)*y2*(-3 + 2*y)*(1 - 4*z + 3*z2))*sd.dz[3]
	+6*x12*x*(-1 + y)*y2*(-1 + z)*z*sd.dxy[3]
	+x12*(1 + 2*x)*(-1 + y)*y2*(1 - 4*z + 3*z2)*sd.dyz[3]
	-(x12*x*y2*(-3 + 2*y)*(1 - 4*z + 3*z2))*sd.dxz[3]
	+x12*x*(-1 + y)*y2*(1 - 4*z + 3*z2)*sd.dxyz[3]
	-6*x12*(1 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z*cd.node4()
	-6*x12*x*y12*(1 + 2*y)*(-1 + z)*z*sd.dx[4]
	-6*x12*(1 + 2*x)*y12*y*(-1 + z)*z*sd.dy[4]
	+x12*(1 + 2*x)*y12*(1 + 2*y)*z*(-2 + 3*z)*sd.dz[4]
	-6*x12*x*y12*y*(-1 + z)*z*sd.dxy[4]
	+x12*(1 + 2*x)*y12*y*z*(-2 + 3*z)*sd.dyz[4]
	+x12*x*y12*(1 + 2*y)*z*(-2 + 3*z)*sd.dxz[4]
	+x12*x*y12*y*z*(-2 + 3*z)*sd.dxyz[4]
	+6*x2*(-3 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z*cd.node5()
	-6*(-1 + x)*x2*y12*(1 + 2*y)*(-1 + z)*z*sd.dx[5]
	+6*x2*(-3 + 2*x)*y12*y*(-1 + z)*z*sd.dy[5]
	-(x2*(-3 + 2*x)*y12*(1 + 2*y)*z*(-2 + 3*z))*sd.dz[5]
	-6*(-1 + x)*x2*y12*y*(-1 + z)*z*sd.dxy[5]
	-(x2*(-3 + 2*x)*y12*y*z*(-2 + 3*z))*sd.dyz[5]
	+(-1 + x)*x2*y12*(1 + 2*y)*z*(-2 + 3*z)*sd.dxz[5]
	+(-1 + x)*x2*y12*y*z*(-2 + 3*z)*sd.dxyz[5]
	-6*x2*(-3 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z*cd.node6()
	+6*(-1 + x)*x2*y2*(-3 + 2*y)*(-1 + z)*z*sd.dx[6]
	+6*x2*(-3 + 2*x)*(-1 + y)*y2*(-1 + z)*z*sd.dy[6]
	+x2*(-3 + 2*x)*y2*(-3 + 2*y)*z*(-2 + 3*z)*sd.dz[6]
	-6*(-1 + x)*x2*(-1 + y)*y2*(-1 + z)*z*sd.dxy[6]
	-(x2*(-3 + 2*x)*(-1 + y)*y2*z*(-2 + 3*z))*sd.dyz[6]
	-((-1 + x)*x2*y2*(-3 + 2*y)*z*(-2 + 3*z))*sd.dxz[6]
	+(-1 + x)*x2*(-1 + y)*y2*z*(-2 + 3*z)*sd.dxyz[6]
	+6*x12*(1 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z*cd.node7()
	+6*x12*x*y2*(-3 + 2*y)*(-1 + z)*z*sd.dx[7]
	-6*x12*(1 + 2*x)*(-1 + y)*y2*(-1 + z)*z*sd.dy[7]
	-(x12*(1 + 2*x)*y2*(-3 + 2*y)*z*(-2 + 3*z))*sd.dz[7]
	-6*x12*x*(-1 + y)*y2*(-1 + z)*z*sd.dxy[7]
	+x12*(1 + 2*x)*(-1 + y)*y2*z*(-2 + 3*z)*sd.dyz[7]
	-(x12*x*y2*(-3 + 2*y)*z*(-2 + 3*z))*sd.dxz[7]
	+x12*x*(-1 + y)*y2*z*(-2 + 3*z)*sd.dxyz[7]);
}

}}} // namespace SCIRun::Core::Basis

#endif // CORE_BASIS_HEXTRICUBICHMTSCALEFACTORSCOMMON_H

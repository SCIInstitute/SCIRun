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


/*
*  BuildBEMatrix.cc:  class to build Boundary Elements matrix
*
*  Written by:
*   Saeed Babaeizadeh
*   Northeastern University
*   January 2006
*/

#include <Core/Algorithms/Legacy/Forward/BuildBEMatrixAlgo.h>

#include <algorithm>
#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <numeric>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/BlockMatrix.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/PointVectorOperators.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Forward;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Forward, FieldNameList);
ALGORITHM_PARAMETER_DEF(Forward, FieldTypeList);
ALGORITHM_PARAMETER_DEF(Forward, BoundaryConditionList);
ALGORITHM_PARAMETER_DEF(Forward, InsideConductivityList);
ALGORITHM_PARAMETER_DEF(Forward, OutsideConductivityList);

void BuildBEMatrixBase::getOmega(
  const Vector& y1,
  const Vector& y2,
  const Vector& y3,
  DenseMatrix& coef)
{
  /*
  This function deals with the analytical solutions of the various integrals in the stiffness matrix
  The function is concerned with the integrals of the linear interpolations functions over the triangles
  and takes care of the solid spherical angle. As in most cases not all values are needed the computation
  is split up in integrals from one surface to another one

  The computational scheme follows the analytical formulas derived by the
  de Munck 1992 (IEEE Trans Biomed Engng, 39-9, pp 986-90)
  */
  const double epsilon  = 1e-12;
  Vector y21 = y2 - y1;
  Vector y32 = y3 - y2;
  Vector y13 = y1 - y3;

  Vector Ny( y1.length() , y2.length() , y3.length() );

  Vector Nyij( y21.length() , y32.length() , y13.length() );

  Vector gamma( 0 , 0 , 0 );
  double NomGamma , DenomGamma;

  NomGamma = Ny[0]*Nyij[0] + Dot(y1,y21);
  DenomGamma = Ny[1]*Nyij[0] + Dot(y2,y21);
  if (fabs(DenomGamma-NomGamma) > epsilon && (DenomGamma != 0) && NomGamma != 0 ){
    gamma[0] = -1/Nyij[0] * log(NomGamma/DenomGamma);
  }
  NomGamma = Ny[1]*Nyij[1] + Dot(y2,y32);
  DenomGamma = Ny[2]*Nyij[1] + Dot(y3,y32);
  if (fabs(DenomGamma-NomGamma) > epsilon && (DenomGamma != 0) && NomGamma != 0 ){
    gamma[1] = -1/Nyij[1] * log(NomGamma/DenomGamma);
  }
  NomGamma = Ny[2]*Nyij[2] + Dot(y3,y13);
  DenomGamma = Ny[0]*Nyij[2] + Dot(y1,y13);
  if (fabs(DenomGamma-NomGamma) > epsilon && (DenomGamma != 0) && NomGamma != 0 ){
    gamma[2] = -1/Nyij[2] * log(NomGamma/DenomGamma);
  }

  double d = Dot( y1, Cross(y2, y3) );

  Vector OmegaVec = (gamma[2]-gamma[0])*y1 + (gamma[0]-gamma[1])*y2 + (gamma[1]-gamma[2])*y3;



  /*
  In order to avoid problems with the arctan used in de Muncks paper
  the result is tested. A problem is that his formula under certain
  circumstances leads to unexpected changes of signs. Hence to avoid
  this, the denominator is checked and 2*pi is added if necessary.
  The problem without the two pi results in the following situation in
  which division of the triangle into three pieces results into
  an opposite sign compared to the spherical angle of the total
  triangle. These cases are rare but existing.
  */

  double Nn=0 , Omega=0 ;
  Nn = Ny[0]*Ny[1]*Ny[2] + Ny[0]*Dot(y2,y3) + Ny[2]*Dot(y1,y2) + Ny[1]*Dot(y3,y1);

  if (Nn > 0)  Omega = 2 * atan( d / Nn );
  if (Nn < 0)  Omega = 2 * atan( d / Nn ) + 2*M_PI ;
  if (Nn == 0)
  {
    if ( d > 0 )
      Omega = M_PI;
    else
      Omega = -M_PI;
  }

  Vector N = Cross(y21, -y13);
  double Zn1 = Dot(Cross(y2, y3) , N);
  double Zn2 = Dot(Cross(y3, y1) , N);
  double Zn3 = Dot(Cross(y1, y2) , N);

  double A2 = N.length2();
  coef(0,0) = (1/A2) * ( Zn1*Omega + d * Dot(y32, OmegaVec) );
  coef(0,1) = (1/A2) * ( Zn2*Omega + d * Dot(y13, OmegaVec) );
  coef(0,2) = (1/A2) * ( Zn3*Omega + d * Dot(y21, OmegaVec) );

}

void  BuildBEMatrixBase::get_cruse_weights(
  const Vector& p1,
  const Vector& p2,
  const Vector& p3,
  double s,
  double r,
  double area,
  DenseMatrix& cruse_weights)
{
  /*
  Inputs: p1,p2,p3= cartesian coordiantes of the triangle vertices ;
  area = triangle area
  Output: cruse_weights = The weighting factors for the 7 Radon points of the triangle
  Format of the cruse_weights matrix
  Radon point 1       Radon Point 2    ...     Radon Point 7
  Vertex 1 ->
  Vertex 2 ->
  Vertex 3 ->

  Set up the local coordinate system around the triangle. This is a 2-D system and
  for ease of use, the x-axis is chosen as the line between the first and second vertex
  of the triangle. From that the vertex of the third point is found in local coordinates.
  */

  // The angle between the F2 and F3, at vertex 1 is (pi - 'alpha').
  Vector fg2 = p1 - p3;
  double fg2_length = fg2.length();

  Vector fg3 = p2 - p1;
  double fg3_length = fg3.length();

  double cos_alpha = - Dot(fg3,fg2) / (fg2_length * fg3_length);
  double sin_alpha = sqrt(1 - cos_alpha * cos_alpha);

  // Now the vertices in local coordinates
  Vector locp1(0 , 0 , 0);
  Vector locp2(fg3_length , 0 , 0);
  Vector locp3(fg2_length * cos_alpha , fg2_length * sin_alpha , 0);

  DenseMatrix Fx(3, 1);
  Fx << locp3[0] - locp2[0],
    locp1[0] - locp3[0],
    locp2[0] - locp1[0];

  DenseMatrix Fy(3, 1);
  Fy << locp3[1] - locp2[1],
    locp1[1] - locp3[1],
    locp2[1] - locp1[1];

  Vector centroid = (locp1 + locp2 + locp3) / 3;
  DenseMatrix loc_radpt_x(1, 7);
  DenseMatrix loc_radpt_y(1, 7);
  loc_radpt_x(0,0) = centroid[0];
  loc_radpt_y(0,0) = centroid[1];
  Vector temp = (1-s) * centroid;
  loc_radpt_x(0,1) = temp[0] + locp1[0]*s;
  loc_radpt_y(0,1) = temp[1] + locp1[1]*s;
  loc_radpt_x(0,2) = temp[0] + locp2[0]*s;
  loc_radpt_y(0,2) = temp[1] + locp2[1]*s;
  loc_radpt_x(0,3) = temp[0] + locp3[0]*s;
  loc_radpt_y(0,3) = temp[1] + locp3[1]*s;
  temp = (1-r) * centroid;
  loc_radpt_x(0,4) = temp[0] + locp1[0]*r;
  loc_radpt_y(0,4) = temp[1] + locp1[1]*r;
  loc_radpt_x(0,5) = temp[0] + locp2[0]*r;
  loc_radpt_y(0,5) = temp[1] + locp2[1]*r;
  loc_radpt_x(0,6) = temp[0] + locp3[0]*r;
  loc_radpt_y(0,6) = temp[1] + locp3[1]*r;

  auto A = (0.5/area) * (Fy * loc_radpt_x - Fx * loc_radpt_y);

  DenseMatrix ones(1, 7, 1.0);
  DenseMatrix E(3, 1);
  /*
  E is a 1X3 matrix: [1st vertex  ;  2nd vertex  ;  3rd vertex]
  E = [1/3 ; 1/3 ; 1/3] + (0.5/area)*(Fy*xmid - Fx*ymid);
  but there is no need to compute the E because by our choice of the
  local coordinates, it is easy to show that the E is always [1 ; 0 ; 0]!
  */
  E << 1,0,0;
  cruse_weights = (E * ones) - A;
}

void BuildBEMatrixBase::get_g_coef(
  const Vector& p1,
  const Vector& p2,
  const Vector& p3,
  const Vector& op,
  double s,
  double r,
  const Vector& centroid,
  DenseMatrix& g_coef)
{
  // Inputs: p1,p2,p3= cartesian coordiantes of the triangle vertices ; op= Observation Point
  // Output: g_coef = G Values (Coefficients) at 7 Radon's points = 1/r
  Vector radpt = centroid - op;
  g_coef(0,0) = 1 / radpt.length();

  Vector temp = centroid * (1-s) - op;
  radpt = temp + p1 * s;
  g_coef(0,1) = 1 / radpt.length();
  radpt = temp + p2 * s;
  g_coef(0,2) = 1 / radpt.length();
  radpt = temp + p3 * s;
  g_coef(0,3) = 1 / radpt.length();

  temp = centroid * (1-r) - op;
  radpt = temp + p1 * r;
  g_coef(0,4) = 1 / radpt.length();
  radpt = temp + p2 * r;
  g_coef(0,5) = 1 / radpt.length();
  radpt = temp + p3 * r;
  g_coef(0,6) = 1 / radpt.length();
}

void BuildBEMatrixBase::bem_sing(
  const Vector& p1,
  const Vector& p2,
  const Vector& p3,
  unsigned int op_n,
  DenseMatrix& g_values,
  double s,
  double r,
  DenseMatrix& R_W)
{
  /*
  This is Jeroen's method, converted from his Matlab code, for dealing with weightings corresponding to singular triangles
  */
  Vector A,B,C,P,BC,BA,AC,AP;
  DenseMatrix WAPB(3,1);
  DenseMatrix WAPC(3,1);
  int one=0,two=1,three=2;

  switch(op_n)
  {
  case 0:
    A = p1; B=p2; C=p3;
    one=0; two=1; three=2;
    break;
  case 1:
    A = p2; B=p3; C=p1;
    one=1; two=2; three=0;
    break;
  case 2:
    A = p3; B=p1; C=p2;
    one=2; two=0; three=1;
    break;
  default:;
  }

  BC=C-B; BA=A-B; AC=C-A;
  double RL = Dot(BA,BC/BC.length())/BC.length();
  P=RL*BC+B;
  AP=A-P;
  double lAP=AP.length();
  double lBC=BC.length();
  double lBP=fabs(RL)*lBC;
  double lCP=fabs(1-RL)*lBC;
  double lAB=BA.length();
  double lAC=AC.length();
  double a,b,c,w,log_term;

  if(fabs(RL) > 0)
  {
    a=lAP; b=lBP; c=lAB;
    log_term=log( (b+c)/a );
    WAPB(0,0)=a/2 * log_term;
    w=1-RL;
    WAPB(1,0)=a* (( a-c)*(-1+w) + b*w*log_term )/(2*b);
    w=RL;
    WAPB(2,0)=a*w *( a-c  +  b*log_term )/(2*b);
  }
  else
  {
    WAPB(0,0)=0; WAPB(1,0)=0; WAPB(2,0)=0;
  }

  if(fabs(RL-1) > 0)
  {
    a = lAP; b = lCP; c = lAC;
    log_term = log( (b+c)/a );
    WAPC(0,0)=a/2 * log_term;
    w = 1-RL;
    WAPC(1,0)=a*w *( a-c  +  b*log_term )/(2*b);
    w = RL;
    WAPC(2,0)=a* (( a-c)*(-1+w) + b*w*log_term )/(2*b);
  }
  else
  {
    WAPC(0,0)=0; WAPC(1,0)=0; WAPC(2,0)=0;
  }

  if(RL<0)
  {
    WAPB(0,0)*=-1.0; WAPB(1,0)*=-1.0; WAPB(2,0)*=-1.0;
  }
  if(RL>1)
  {
    WAPC(0,0)*=-1.0; WAPC(1,0)*=-1.0; WAPC(2,0)*=-1.0;
  }

  g_values(one,0) = WAPB(0,0) + WAPC(0,0);
  g_values(two,0) = WAPB(1,0) + WAPC(1,0);
  g_values(three,0) = WAPB(2,0) + WAPC(2,0);
}

void BuildBEMatrixBase::get_auto_g(
  const Vector& p1,
  const Vector& p2,
  const Vector& p3,
  unsigned int op_n,
  DenseMatrix& g_values,
  double s,
  double r,
  DenseMatrix& R_W)
{
  /*
  A routine to solve the Auto G-parameter integral for a triangle from
  a "closed" observation point.
  The scheme is the standard one for all the BEM routines.
  Input are the observation point and triangle co-ordinates
  Input:
  op_n = observation point number (1, 2 or 3)
  p1,p2,p3 = triangle co-ordinates (REAL)

  Output:
  g_values
  = the total values for the 1/r integral for
  each of the subtriangles associated with each
  integration triangle vertex about the observation
  point defined by the calling
  program.
  */

  Vector p5 = (p1 + p2) / 2;
  Vector p6 = (p2 + p3) / 2;
  Vector p4 = (p1 + p3) / 2;
  Vector ctroid = (p1 + p2 + p3) / 3;
  Vector op;

  switch(op_n)
  {
  case 0:
    op = p1;
    g_values(0,0) = get_new_auto_g(op, p5, p4) + do_radon_g(p5, ctroid, p4, op, s, r, R_W);
    g_values(1,0) = do_radon_g(p2, p6, p5, op, s, r, R_W) + do_radon_g(p5, p6, ctroid, op, s, r, R_W);
    g_values(2,0) = do_radon_g(p3, p4, p6, op, s, r, R_W) + do_radon_g(p4, ctroid, p6, op, s, r, R_W);
    break;
  case 1:
    op = p2;
    g_values(0,0) = do_radon_g(p1, p5, p4, op, s, r, R_W) + do_radon_g(p5, ctroid, p4, op, s, r, R_W);
    g_values(1,0) = get_new_auto_g(op, p6, p5) + do_radon_g(p5, p6, ctroid, op, s, r, R_W);
    g_values(2,0) = do_radon_g(p3, p4, p6, op, s, r, R_W) + do_radon_g(p4, ctroid, p6, op, s, r, R_W);
    break;
  case 2:
    op = p3;
    g_values(0,0) = do_radon_g(p1, p5, p4, op, s, r, R_W) + do_radon_g(p5, ctroid, p4, op, s, r, R_W);
    g_values(1,0) = do_radon_g(p2, p6, p5, op, s, r, R_W) + do_radon_g(p5, p6, ctroid, op, s, r, R_W);
    g_values(2,0) = get_new_auto_g(op, p4, p6) + do_radon_g(p4, ctroid, p6, op, s, r, R_W);
    break;
  }
}

double BuildBEMatrixBase::get_new_auto_g(
  const Vector& op,
  const Vector& p2,
  const Vector& p3)
{
  //  Inputs: op,p2,p3= cartesian coordiantes of the triangle vertices ; op= Observation Point
  //  Output: g1 = G value for the triangle for "auto_g"
  //  This function is called from get_auto_g.m

  double delta_min = 0.00001;
  unsigned int max_number_of_divisions = 256;

  Vector a = p2 - op; double a_mag = a.length();
  Vector b = p3 - p2; double b_mag = b.length();
  Vector c = op - p3; double c_mag = c.length();

  Vector aV = Cross(p2 - op, p3 - p2)*0.5;
  double area = aV.length();
  double area2 = 2.0*area;
  double h = (area2) / b_mag;
  double alfa=0;  if (h<a_mag) alfa = acos(h/a_mag);
  double AC = a_mag*c_mag;
  double teta = 0; if (area2<=AC) teta = asin( area2 / AC );

  unsigned int nod = 1;
  double sai_old = sqrt(area2 * teta);
  double delta = 1;

  double gama, gama_j, rhoj_1, rhoj, sum, sai_new=0;

  while( (delta >= delta_min) && (nod <= max_number_of_divisions) )
  {
    nod = 2*nod;
    gama = teta / nod;
    sum = 0;
    gama_j = 0;
    rhoj_1 = a_mag;
    for ( unsigned int j = 1; j <= nod; j++)
    {
      gama_j = gama_j + gama;
      rhoj = h / cos(alfa - gama_j);
      sum = sum + sqrt( std::fabs(rhoj * rhoj_1) );
      rhoj_1 = rhoj;
    }
    sai_new = sum * sqrt(std::fabs(gama * sin(gama)));
    delta = 0;
    if (sai_new + sai_old)
      delta = std::fabs((sai_new - sai_old) / (sai_new + sai_old));
    sai_old = sai_new;
  }
  return sai_new;
}


double BuildBEMatrixBase::do_radon_g(
  const Vector& p1,
  const Vector& p2,
  const Vector& p3,
  const Vector& op,
  double s,
  double r,
  DenseMatrix& R_W)
{
  //  Inputs: p1,p2,p3= cartesian coordiantes of the triangle vertices ; op= Observation Point
  //  Output: g2 = G value for the triangle for "auto_g"
  //  This function is called from get_auto_g.m

  Vector centroid = (p1 + p2 + p3) / 3;

  DenseMatrix g_coef(1, 7);
  get_g_coef(p1, p2, p3, op, s, r, centroid, g_coef);

  double g2 = 0;
  for (int i=0; i<7; i++)   g2 = g2 + g_coef(0,i)*R_W(0,i);

  Vector aV = Cross(p2 - p1, p3 - p2)*0.5;

  return g2 * aV.length();
}

class BuildBEMatrixBaseCompute : public BuildBEMatrixBase
{
public:
  template <class MatrixType>
  static void make_auto_P_compute(VMesh* hsurf, MatrixType& auto_P, double in_cond, double out_cond, double op_cond);

  template <class MatrixType>
  static void make_cross_P_compute(VMesh* hsurf1, VMesh* hsurf2, MatrixType& cross_P, double in_cond, double out_cond, double op_cond);

  template <class MatrixType>
  static void make_auto_G_compute(VMesh* hsurf, MatrixType& auto_G, double in_cond, double out_cond, double op_cond, const std::vector<double>& avInn);

  template <class MatrixType>
  static void make_cross_G_compute( VMesh*,
  VMesh*,
  MatrixType&,
  double,
  double,
  double,
  const std::vector<double>& );
};

void BuildBEMatrixBase::make_auto_G_allocate(VMesh* hsurf, DenseMatrixHandle &h_GG_)
{
  auto nnodes = numNodes(hsurf);
  h_GG_.reset(new DenseMatrix(nnodes, nnodes, 0.0));
}

void BuildBEMatrixBase::make_auto_G(VMesh* hsurf, DenseMatrixHandle &h_GG_,
double in_cond, double out_cond, double op_cond, const std::vector<double>& avInn)
{
  make_auto_G_allocate(hsurf, h_GG_);
  BuildBEMatrixBaseCompute::make_auto_G_compute(hsurf, *h_GG_, in_cond, out_cond, op_cond, avInn);
}

template <class MatrixType>
void BuildBEMatrixBaseCompute::make_auto_G_compute(VMesh* hsurf, MatrixType& auto_G,
  double in_cond, double out_cond, double op_cond, const std::vector<double>& avInn)
{
  //const double mult = 1/(2*M_PI)*((out_cond - in_cond)/op_cond);  // op_cond=out_cond for all the surfaces but the outermost surface which in op_cond=in_cond
  const double mult = 1/(4*M_PI)*(out_cond - in_cond);  // op_cond=out_cond for all the surfaces but the outermost surface which in op_cond=in_cond

  VMesh::Node::array_type nodes;

  VMesh::Node::iterator ni, nie;
  VMesh::Face::iterator fi, fie;
  DenseMatrix cruse_weights(3, 7);
  DenseMatrix g_coef(1, 7);
  DenseMatrix R_W(1,7); // Radon Points Weights
  DenseMatrix temp(1,7);
  DenseMatrix g_values(3, 1);

  double area;

  double sqrt15 = sqrt(15.0);
  //R_W(0,0) = 9/40; // <- Burak! FIX ME!
  R_W(0,0) = 9.0/40.0;
  R_W(0,1) = (155 + sqrt15) / 1200;
  R_W(0,2) = R_W(0,1);
  R_W(0,3) = R_W(0,1);
  R_W(0,4) = (155 - sqrt15) / 1200;
  R_W(0,5) = R_W(0,4);
  R_W(0,6) = R_W(0,4);

  double s = (1 - sqrt15) / 7;
  double r = (1 + sqrt15) / 7;


  hsurf->begin(fi); hsurf->end(fie);
  for (; fi != fie; ++fi)
  { //! find contributions from every triangle
    hsurf->get_nodes(nodes, *fi);
    Vector p1(hsurf->get_point(nodes[0]));
    Vector p2(hsurf->get_point(nodes[1]));
    Vector p3(hsurf->get_point(nodes[2]));

    area = avInn[*fi];

    get_cruse_weights(p1, p2, p3, s, r, area, cruse_weights);
    Vector centroid = (p1 + p2 + p3) / 3.0;
    hsurf->begin(ni); hsurf->end(nie);
    for (; ni != nie; ++ni)
    { //! for every node
      VMesh::Node::index_type ppi = *ni;
      Vector op(hsurf->get_point(ppi));

      if (ppi == nodes[0])       bem_sing(p1, p2, p3, 0, g_values, s, r, R_W);
      else if (ppi == nodes[1])       bem_sing(p1, p2, p3, 1, g_values, s, r, R_W);
      else if (ppi == nodes[2])       bem_sing(p1, p2, p3, 2, g_values, s, r, R_W);
      else
      {
        get_g_coef(p1, p2, p3, op, s, r, centroid, g_coef);

        for (int i=0; i<7; i++)  temp(0,i) = g_coef(0,i)*R_W(0,i);

        g_values = area * (cruse_weights * temp.transpose());
      } // else

      for (int i=0; i<3; ++i)
        auto_G(ppi, nodes[i])+=g_values(i,0)*mult;
    }
  }
}

void BuildBEMatrixBase::make_cross_G_allocate(VMesh* hsurf1, VMesh* hsurf2, DenseMatrixHandle &h_GG_)
{
  h_GG_.reset(new DenseMatrix(numNodes(hsurf1), numNodes(hsurf2), 0.0));
}

void BuildBEMatrixBase::make_cross_G(VMesh* hsurf1, VMesh* hsurf2, DenseMatrixHandle &h_GG_,
  double in_cond, double out_cond, double op_cond, const std::vector<double>& avInn)
{
  make_cross_G_allocate(hsurf1, hsurf2, h_GG_);
  BuildBEMatrixBaseCompute::make_cross_G_compute(hsurf1, hsurf2, *h_GG_, in_cond, out_cond, op_cond, avInn);
}

template <class MatrixType>
void BuildBEMatrixBaseCompute::make_cross_G_compute(VMesh* hsurf1, VMesh* hsurf2, MatrixType& cross_G,
  double in_cond, double out_cond, double op_cond, const std::vector<double>& avInn)
{
  const double mult = 1/(4*M_PI)*(out_cond - in_cond);
  //   out_cond and in_cond belong to hsurf2 and op_cond is the out_cond of hsurf1 for all the surfaces but the outermost surface which in op_cond=in_cond

  VMesh::Node::array_type nodes;

  VMesh::Node::iterator  ni, nie;
  VMesh::Face::iterator  fi, fie;

  DenseMatrix cruse_weights(3, 7);
  DenseMatrix g_coef(1, 7);
  DenseMatrix R_W(1,7); // Radon Points Weights
  DenseMatrix temp(1,7);
  DenseMatrix g_values(3, 1);

  double area;

  double sqrt15 = sqrt(15.0);
  //R_W(0,0) = 9/40; // <- Burak! FIX ME!
  R_W(0,0) = 9.0/40.0;
  R_W(0,1) = (155 + sqrt15) / 1200;
  R_W(0,2) = R_W(0,1);
  R_W(0,3) = R_W(0,1);
  R_W(0,4) = (155 - sqrt15) / 1200;
  R_W(0,5) = R_W(0,4);
  R_W(0,6) = R_W(0,4);

  double s = (1 - sqrt15) / 7;
  double r = (1 + sqrt15) / 7;

  hsurf2->begin(fi); hsurf2->end(fie);
  for (; fi != fie; ++fi)
  { //! find contributions from every triangle
    hsurf2->get_nodes(nodes, *fi);
    Vector p1(hsurf2->get_point(nodes[0]));
    Vector p2(hsurf2->get_point(nodes[1]));
    Vector p3(hsurf2->get_point(nodes[2]));

    area = avInn[*fi];

    get_cruse_weights(p1, p2, p3, s, r, area, cruse_weights);
    Vector centroid = (p1 + p2 + p3) / 3.0;

    hsurf1->begin(ni); hsurf1->end(nie);
    for (; ni != nie; ++ni)
    { //! for every node
      VMesh::Node::index_type ppi = *ni;
      Vector op(hsurf1->get_point(ppi));
      get_g_coef(p1, p2, p3, op, s, r, centroid, g_coef);

      for (int i=0; i<7; i++)  temp(0,i) = g_coef(0,i)*R_W(0,i);

      g_values = area * (cruse_weights * temp.transpose());

      for (int i=0; i<3; ++i)
        cross_G(ppi, nodes[i])+=g_values(i,0)*mult;
    }
  }
}

void BuildBEMatrixBase::make_cross_P_allocate(VMesh* hsurf1, VMesh* hsurf2, DenseMatrixHandle &h_PP_)
{
  h_PP_.reset(new DenseMatrix(numNodes(hsurf1), numNodes(hsurf2), 0.0));
}

void BuildBEMatrixBase::make_cross_P(VMesh* hsurf1, VMesh* hsurf2, DenseMatrixHandle &h_PP_,
  double in_cond, double out_cond, double op_cond)
{
  make_cross_P_allocate(hsurf1, hsurf2, h_PP_);
  BuildBEMatrixBaseCompute::make_cross_P_compute(hsurf1, hsurf2, *h_PP_, in_cond, out_cond, op_cond);
}

template <class MatrixType>
void BuildBEMatrixBaseCompute::make_cross_P_compute(VMesh* hsurf1, VMesh* hsurf2, MatrixType& cross_P, double in_cond, double out_cond, double op_cond)
{
  const double mult = 1/(4*M_PI)*(out_cond - in_cond);
  //   out_cond and in_cond belong to hsurf2 and op_cond is the out_cond of hsurf1 for all the surfaces but the outermost surface which in op_cond=in_cond
  VMesh::Node::array_type nodes;
  DenseMatrix coef(1, 3);
  int i;

  VMesh::Node::iterator  ni, nie;
  VMesh::Face::iterator  fi, fie;

  hsurf1->begin(ni); hsurf1->end(nie);
  for (; ni != nie; ++ni){ //! for every node
    VMesh::Node::index_type ppi = *ni;
    Point pp = hsurf1->get_point(ppi);

    hsurf2->begin(fi); hsurf2->end(fie);
    for (; fi != fie; ++fi){ //! find contributions from every triangle

      hsurf2->get_nodes(nodes, *fi);
      Vector v1 = hsurf2->get_point(nodes[0]) - pp;
      Vector v2 = hsurf2->get_point(nodes[1]) - pp;
      Vector v3 = hsurf2->get_point(nodes[2]) - pp;

      getOmega(v1, v2, v3, coef);

      for (i=0; i<3; ++i)
        cross_P(ppi, nodes[i])-=coef(0,i)*mult;
    }
  }
}

void BuildBEMatrixBase::make_auto_P_allocate(VMesh* hsurf, DenseMatrixHandle &h_PP_)
{
  auto nnodes = numNodes(hsurf);
  h_PP_.reset(new DenseMatrix(nnodes, nnodes, 0.0));
}

int BuildBEMatrixBase::numNodes(FieldHandle f)
{
  return numNodes(f->vmesh());
}

int BuildBEMatrixBase::numNodes(VMesh* hsurf)
{
  VMesh::Node::size_type nsize;
  hsurf->size(nsize);
  return static_cast<int>(nsize);
}

template <class MatrixType>
void BuildBEMatrixBaseCompute::make_auto_P_compute(VMesh* hsurf, MatrixType& auto_P, double in_cond, double out_cond, double op_cond)
{
  auto nnodes = auto_P.rows();



  //const double mult = 1/(2*M_PI)*((out_cond - in_cond)/op_cond);  // op_cond=out_cond for all the surfaces but the outermost surface which in op_cond=in_cond
  const double mult = 1/(4*M_PI)*(out_cond - in_cond);

  VMesh::Node::array_type nodes;
  DenseMatrix coef(1, 3);

  VMesh::Node::iterator ni, nie;
  VMesh::Face::iterator fi, fie;

  unsigned int i;

  hsurf->begin(ni); hsurf->end(nie);

  for (; ni != nie; ++ni){ //! for every node
    VMesh::Node::index_type ppi = *ni;
    Point pp = hsurf->get_point(ppi);

    hsurf->begin(fi); hsurf->end(fie);

    for (; fi != fie; ++fi) { //! find contributions from every triangle

      hsurf->get_nodes(nodes, *fi);
      if (ppi!=nodes[0] && ppi!=nodes[1] && ppi!=nodes[2]){
        Vector v1 = hsurf->get_point(nodes[0]) - pp;
        Vector v2 = hsurf->get_point(nodes[1]) - pp;
        Vector v3 = hsurf->get_point(nodes[2]) - pp;

        getOmega(v1, v2, v3, coef);

        for (i=0; i<3; ++i)
          auto_P(ppi, nodes[i])-=coef(0,i)*mult;
      }
    }
  }

  //! accounting for autosolid angle
  auto sumOfRows = auto_P.rowwise().sum().eval();
  for (i=0; i<nnodes; ++i)
  {
    auto_P(i,i) = out_cond - sumOfRows(i);
  }
}

void BuildBEMatrixBase::make_auto_P(VMesh* hsurf, DenseMatrixHandle &h_PP_,
  double in_cond, double out_cond, double op_cond)
{
  make_auto_P_allocate(hsurf, h_PP_);
  BuildBEMatrixBaseCompute::make_auto_P_compute(hsurf, *h_PP_, in_cond, out_cond, op_cond);
}

// precalculate triangles area
void BuildBEMatrixBase::pre_calc_tri_areas(VMesh* hsurf, std::vector<double>& areaV){

  VMesh::Face::iterator  fi, fie;

  hsurf->begin(fi);
  hsurf->end(fie);
  for (; fi != fie; ++fi)
    areaV.push_back(hsurf->get_area(*fi));
}

// C++ized MollerTrumbore97 Ray Triangle intersection test.
bool BuildBEMatrixBase::ray_triangle_intersect(double &t,
  const Point &point,
  const Vector &dir,
  const Point &p0,
  const Point &p1,
  const Point &p2)
{
  // Find vectors for two edges sharing p0.
  const Vector edge1 = p1 - p0;
  const Vector edge2 = p2 - p0;

  // begin calculating determinant - also used to calculate U parameter.
  const Vector pvec = Cross(dir, edge2);

  // if determinant is near zero, ray lies in plane of triangle.
  const double det = Dot(edge1, pvec);
  const double EPSILON = 1.0e-6;
  if (det > -EPSILON && det < EPSILON)
  {
    return false;
  }
  const double inv_det = 1.0 / det;

  // Calculate distance from vert0 to ray origin.
  const Vector tvec = point - p0;

  // Calculate U parameter and test bounds.
  const double u = Dot(tvec, pvec) * inv_det;
  if (u < 0.0 || u > 1.0)
  {
    return false;
  }

  // Prepare to test V parameter.
  const Vector qvec = Cross(tvec, edge1);

  // Calculate V parameter and test bounds.
  const double v = Dot(dir, qvec) * inv_det;
  if (v < 0.0 || u + v > 1.0)
  {
    return false;
  }

  // Calculate t, ray intersects triangle.
  t = Dot(edge2, qvec) * inv_det;

  return true;
}

void BuildBEMatrixBase::compute_intersections(std::vector<std::pair<double, int> >
  &results,
  const VMesh* mesh,
  const Point &p, const Vector &v,
  int marker)
{
  VMesh::Face::iterator itr, eitr;
  mesh->begin(itr);
  mesh->end(eitr);
  double t;
  while (itr != eitr)
  {
    VMesh::Node::array_type nodes;
    mesh->get_nodes(nodes, *itr);
    Point p0, p1, p2;
    mesh->get_center(p0, nodes[0]);
    mesh->get_center(p1, nodes[1]);
    mesh->get_center(p2, nodes[2]);
    if (ray_triangle_intersect(t, p, v, p0, p1, p2))
    {
      results.push_back(std::make_pair(t, marker));
    }
    ++itr;
  }
}

static bool
  pair_less(const std::pair<double, int> &a,
  const std::pair<double, int> &b)
{
  return a.first < b.first;
}

int BuildBEMatrixBase::compute_parent(const std::vector<VMesh*> &meshes, int index)
{
  Point point;
  meshes[index]->get_center(point, VMesh::Node::index_type(0));
  Vector dir(1.0, 1.0, 1.0);
  std::vector<std::pair<double, int> > intersections;

  unsigned int i;
  for (i = 0; i < (unsigned int)meshes.size(); i++)
  {
    compute_intersections(intersections, meshes[i], point, dir, i);
  }

  std::sort(intersections.begin(), intersections.end(), pair_less);

  std::vector<int> counts(meshes.size(), 0);
  for (i = 0; i < intersections.size(); i++)
  {
    if (intersections[i].second == index)
    {
      // First odd count is parent.
      for (int j = i-1; j >= 0; j--)
      {
        // TODO: unusual odd/even number test?
        if (counts[intersections[j].second] & 1)
        {
          return intersections[j].second;
        }
      }
      // No odd parent, is outside.
      return static_cast<int>( meshes.size() );
    }
    counts[intersections[i].second]++;
  }

  // Indeterminate, we should intersect with ourselves.
  return static_cast<int>( meshes.size() );
}

bool BuildBEMatrixBase::compute_nesting(std::vector<int> &nesting, const std::vector<VMesh*> &meshes)
{
  nesting.resize(meshes.size());

  unsigned int i;
  for (i = 0; i < (unsigned int)meshes.size(); i++)
  {
    nesting[i] = compute_parent(meshes, i);
  }

  return true;
}

class SurfaceAndPoints : public BEMAlgoImpl, public BuildBEMatrixBaseCompute
{
public:
  virtual MatrixHandle compute(const bemfield_vector& fields) const override;
};

class SurfaceToSurface : public BEMAlgoImpl, public BuildBEMatrixBaseCompute
{
public:
  virtual MatrixHandle compute(const bemfield_vector& fields) const override;
};

BEMAlgoPtr BEMAlgoImplFactory::create(const bemfield_vector& fields)
{
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  // Check for special case where the potentials need to be evaluated at the nodes of a lead
  // This case assumes the first input is the surface mesh and the second is the location of the
  // nodes

  const bemfield_vector::size_type SPECIAL_CASE_LEN = 2;

  if ( fields.size() == SPECIAL_CASE_LEN )
  {
    VMesh *surface, *nodes;
    int surfcount=0, pointcloudcount=0;

    bool meets_conditions = true;

    for (bemfield_vector::size_type i = 0; i < SPECIAL_CASE_LEN; i++)
    {
      if (fields[i].surface)
      {
        surface = fields[i].field_->vmesh();
        if (! surface->is_trisurfmesh() ) meets_conditions = false;
        surfcount++;
      }
      else
      {
        nodes = fields[i].field_->vmesh();
        if (! ( nodes->is_pointcloudmesh() ) || nodes->is_curvemesh() )
        {
          meets_conditions = false;
        }
        pointcloudcount++;
      }
    }

    if ( (surfcount == 0) || (pointcloudcount == 0) )
    {
      meets_conditions = false;
    }

    // If all of the checks above don't flag meets_conditions as false,
    // return a value that indicates the algorithm to use is the surface-to-nodes case
    if ( meets_conditions )
      return boost::make_shared<SurfaceAndPoints>();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Check for case where all inputs are triangle surfaces, and there is at least one source and
  // one measurement surface

  bool allsurfaces=true, hasmeasurementsurf=false, hassourcesurf=false;

  for (bemfield_vector::size_type i = 0; i < fields.size(); i++)
  {
    // if the current field is not marked to be used as a surface OR it's not of trisurfmesh type, this algorithm does not apply
    if ( (! fields[i].field_->vmesh()->is_trisurfmesh()) || (! fields[i].surface) )
    {
      allsurfaces=false;
      break;
    }

    if (fields[i].measurement) hasmeasurementsurf = true;
    if (fields[i].source) hassourcesurf = true;
  }

  // if all fields are surfaces, there exists a measurement and a source surface, then use the surface-to-surface algorithm... else fail
  if (allsurfaces && hasmeasurementsurf && hassourcesurf)
  {
    return boost::make_shared<SurfaceToSurface>();
  }
  else
  {
    return nullptr;
  }
}

static void printInfo(const DenseMatrix& m, const std::string& name)
{
#if 0
  std::cout << name << ": " << m.rows() << " x " << m.cols() << std::endl;
  std::cout << name << " min: " << m.minCoeff() << std::endl;
  std::cout << name << " max: " << m.maxCoeff() << std::endl;
#endif
}

MatrixHandle SurfaceToSurface::compute(const bemfield_vector& fields) const
{
  // Math for surface-to-surface BEM algorithm (based on Jeroen Stinstra's BEM Matlab code that's part of SCIRun)
  // -------------------------------------------------------------------------------------------------------------
  // EE = matrix relating potentials on surfaces to potentials on other surfaces
  // EJ = matrix relating current density on surfaces (normal to surface) to potentials on surfaces
  // u  = potentials on the surfaces
  // j  = current density normal to the surfaces
  //
  // General equation: EE*u + EJ*j = (dipolar sources not on the surfaces)
  // Assuming all sources are on surfaces: EE*u + EJ*j = 0
  // (below assumes that measurement=Neumann boundary conditions and source=Dirichlet boundary conditions)
  //
  // s = source indices
  // m = measurement indices
  //
  // Pmm = EE(m,m)
  // Pss = EE(s,s)
  // Pms = EE(m,s)
  // Psm = EE(s,m)
  //
  // Gms = EJ(m,s)
  // Gss = EJ(s,s)
  //
  // After some block-matrix math to eliminate j from the equation and find T s.t. u(m)=T*u(s), we get:
  // iGss = inv(Gss)
  // T = inv(Pmm - Gms*iGss*Psm)*(Gms*iGss*Pss - Pms)
  //

  const size_t Nfields = fields.size();
  double op_cond=0.0; // op_cond is not used in this formulation -- someone needs to check this math and make a better decision about how to handle this value below

  // Count the number of fields that have been specified as being "sources" or "measurements" (and keep track of indices)
  int Nsources = 0;
  std::vector<int> sourcefieldindices;
  int Nmeasurements = 0;
  std::vector<int> measurementfieldindices;

  for(int i=0; i < Nfields; i++)
  {
    if(fields[i].source)
    {
      Nsources++;
      sourcefieldindices.push_back(i);
    }
    else if(fields[i].measurement)
    {
      Nmeasurements++;
      measurementfieldindices.push_back(i);
    }
  }

  std::vector<int> fieldNodeSize(fields.size());
  std::transform(fields.begin(), fields.end(), fieldNodeSize.begin(), [](const bemfield& f) { return numNodes(f.field_); } );
  DenseBlockMatrix EE(fieldNodeSize, fieldNodeSize);

  // Calculate EE in block matrix form
  for(int i = 0; i < Nfields; i++)
  {
    for(int j = 0; j < Nfields; j++)
    {
      if (i == j)
      {
        auto block = EE.blockRef(i, j);
        make_auto_P_compute(fields[i].field_->vmesh(), block, fields[i].insideconductivity, fields[i].outsideconductivity, op_cond);
      }
      else
      {
        auto block = EE.blockRef(i, j);
        make_cross_P_compute(fields[i].field_->vmesh(), fields[j].field_->vmesh(), block, fields[j].insideconductivity, fields[j].outsideconductivity, op_cond);
      }
    }
  }

  printInfo(EE.matrix(), "EE");

  std::vector<int> sourceFieldNodeSize(sourcefieldindices.size());
  auto sourceFields = fields | boost::adaptors::filtered([](const bemfield& f) { return f.source; });
  //following doesn't compile in VS2010, but does in clang: enable it after upgrading to 2013
  //auto transformer = [this](const bemfield& f) -> int { return numNodes(f.field_); };
  //auto trans = filt | boost::adaptors::transformed(transformer);
  //boost::copy(trans, sourceFieldNodeSize.begin());
  std::transform(sourceFields.begin(), sourceFields.end(), sourceFieldNodeSize.begin(), [](const bemfield& f) { return numNodes(f.field_); } );

  DenseBlockMatrix EJ(fieldNodeSize, sourceFieldNodeSize);

  // Calculate EJ(:,s) in block matrix form
  // ***NOTE THE CHANGE IN INDEXING!!!***
  // (The indices of block columns of EJ correspond to field indices according to "sourcefieldindices", and this affects everything with EJ below this point too!)
  for(int j = 0; j < Nsources; j++)
  {
    // Precalculate triangle areas for this source field/surface
    std::vector<double> triangleareas;
    pre_calc_tri_areas(fields[sourcefieldindices[j]].field_->vmesh(), triangleareas);

    for(int i = 0; i < Nfields; i++)
    {
      if (i == sourcefieldindices[j])
      {
        auto block = EJ.blockRef(i,j);
        make_auto_G_compute(fields[i].field_->vmesh(), block, fields[i].insideconductivity, fields[i].outsideconductivity, op_cond, triangleareas);
      }
      else
      {
        auto block = EJ.blockRef(i,j);
        make_cross_G_compute(fields[i].field_->vmesh(), fields[sourcefieldindices[j]].field_->vmesh(), block, fields[j].insideconductivity, fields[j].outsideconductivity, op_cond, triangleareas);
      }
    }
  }

  printInfo(EJ.matrix(), "EJ");

  // This needs to be checked.  It was taken out because the deflation was producing errors
  // Jeroen's matlab code, which was the basis of this code, only does a defation in test cases.

  // Perform deflation on EE matrix
  //const double deflationconstant = 1.0/EE.matrix().ncols();
  //EE.matrix() = EE.matrix().array() + deflationconstant;

  std::vector<int> measurementNodeSize(measurementfieldindices.size());
  auto measFields = fields | boost::adaptors::filtered([](const bemfield& f) { return f.measurement; });
  //following doesn't compile in VS2010, but does in clang: enable it after upgrading to 2013
  //auto transformer = [this](const bemfield& f) -> int { return numNodes(f.field_); };
  //auto trans = filt | boost::adaptors::transformed(transformer);
  //boost::copy(trans, sourceFieldNodeSize.begin());
  std::transform(measFields.begin(), measFields.end(), measurementNodeSize.begin(), [](const bemfield& f) { return numNodes(f.field_); } );

  // Split EE apart into Pmm, Pss, Pms, and Psm
  // -----------------------------------------------
  // Pmm:
  DenseBlockMatrix Pmm(measurementNodeSize, measurementNodeSize);
  for(int i = 0; i < Nmeasurements; i++)
  {
    for(int j = 0; j < Nmeasurements; j++)
    {
      Pmm.blockRef(i,j) = EE.blockRef(measurementfieldindices[i], measurementfieldindices[j]);
    }
  }
  printInfo(Pmm.matrix(), "Pmm");

  // Pss:
  DenseBlockMatrix Pss(sourceFieldNodeSize, sourceFieldNodeSize);
  for(int i = 0; i < Nsources; i++)
  {
    for(int j = 0; j < Nsources; j++)
    {
      Pss.blockRef(i,j) = EE.blockRef(sourcefieldindices[i],sourcefieldindices[j]);
    }
  }
  printInfo(Pss.matrix(), "Pss");

  // Pms:
  DenseBlockMatrix Pms(measurementNodeSize, sourceFieldNodeSize);
  for(int i = 0; i < Nmeasurements; i++)
  {
    for(int j = 0; j < Nsources; j++)
    {
      Pms.blockRef(i,j) = EE.blockRef(measurementfieldindices[i],sourcefieldindices[j]);
    }
  }
  printInfo(Pms.matrix(), "Pms");


  // Psm:
  DenseBlockMatrix Psm(sourceFieldNodeSize, measurementNodeSize);
  for(int i = 0; i < Nsources; i++)
  {
    for(int j = 0; j < Nmeasurements; j++)
    {
      Psm.blockRef(i,j) = EE.blockRef(sourcefieldindices[i],measurementfieldindices[j]);
    }
  }
  printInfo(Psm.matrix(), "Psm");

  // Split EJ apart into Gms and Gss (see ALL-CAPS note above about differences in block row vs column indexing in EJ matrix)
  // -----------------------------------------------
  // Gms:
  DenseBlockMatrix Gms(measurementNodeSize, sourceFieldNodeSize);
  for(int i = 0; i < Nmeasurements; i++)
  {
    for(int j = 0; j < Nsources; j++)
    {
      Gms.blockRef(i,j) = EJ.blockRef(measurementfieldindices[i],j);
    }
  }
  printInfo(Gms.matrix(), "Gms");

  // Gss:
  DenseBlockMatrix Gss(sourceFieldNodeSize, sourceFieldNodeSize);
  for(int i = 0; i < Nsources; i++)
  {
    for(int j = 0; j < Nsources; j++)
    {
      Gss.blockRef(i,j) = EJ.blockRef(sourcefieldindices[i],j);
    }
  }
  printInfo(Gss.matrix(), "Gss");

  // TODO: add deflation step

  // Compute T here (see math in comments above)
  // TransferMatrix = T = inv(Pmm - Gms*iGss*Psm)*(Gms*iGss*Pss - Pms) = inv(C)*D

  auto Y = Gms.matrix() * Gss.matrix().inverse();
  auto C = Pmm.matrix() - Y * Psm.matrix();
  auto D = Y * Pss.matrix() - Pms.matrix();

  auto T = C.inverse() * D; // T = inv(C)*D
  return boost::make_shared<DenseMatrix>(T);

  //This could be done on one line (see below), but Y (see above) would need to be calculated twice:
  //MatrixHandle TransferMatrix1 = inv(Pmm - Gms * Gss * Psm) * (Gms * Gss * Pss - Pms);
}


MatrixHandle SurfaceAndPoints::compute(const bemfield_vector& fields) const
{
  // NOTE: This is Jeroen's code that has been adapted to fit the new module structure
  //
  // Math:
  // The boundary element formulation is based on Matlab code
  // bemMatrixPP2.m, which can be found in the matlab package

  // The BEM formulation assumes the following matrix equations
  // P_surf_surf * PHI_surf + G_surf_surf * J_surf =  sources_in_volume
  //
  // PHI_surf are the potentials on the surface
  // J_surf are the currents passing perpendicular to the surface
  // sources_in_volume is empty in this case
  //
  // P_surf_surf is the matrix that connects the potentials at the nodes to the integral over the
  // potential at the surface. Its terms consist of Green's function ( 1/ ( 4pi*||r-r'|| ) ) over
  // the surface of each element. As this integral becomes singular for a node and a triangle that
  // share a corner node, we use a trick to avoid computing this integral as we know that the
  // the system should reference potential invariant. Hence the rows of the matrix need to sum to
  // to zero
  //
  // G_surf_surf is the matrix that connects the potentials at the nodes to the integral over the
  // currents flowing through the surface.
  //
  // The second equation that we use is the expression of the potentials at an arbitrary point
  // to the potentials at the surface and the current flowing through the surface
  //
  // PHI_nodes = P_nodes_surf * PHI_surf + G_nodes_surf * J_surf
  //
  // Here matrix P_nodes_surf is the matrix that projects the contribution of the potentials of the
  // surface to the nodes within the volume
  //
  // Here G_nodes_surf is the matrix that projects the contribution of the currents flowing through
  // the surface to the nodes within the volume
  //
  // Adding both equations together will result in
  //
  // PHI_nodes = P_nodes_surf* PHI_surf - G_nodes_surf * inv( G_surf_surf) * P_surf_surf * PHI_surf
  //
  // In other words the transfer matrix is
  // P_nodes_surf - G_nodes_surf * inv( G_surf_surf) * P_surf_surf

  VMesh *nodes = 0;
  VMesh *surface = 0;

  for (int i=0; i<2; i++)
  {
    if (fields[i].surface)
      surface = fields[i].field_->vmesh();
    else
      nodes = fields[i].field_->vmesh();
  }

  DenseMatrixHandle Pss;
  DenseMatrixHandle Gss;
  DenseMatrixHandle Pns;
  DenseMatrixHandle Gns;
  make_auto_P( surface, Pss, 1.0, 0.0, 1.0 );
  make_cross_P( nodes, surface, Pns, 1.0, 0.0, 1.0 );

  std::vector<double> area;
  pre_calc_tri_areas( surface, area );

  make_auto_G( surface, Gss, 1.0, 0.0, 1.0, area );
  make_cross_G( nodes, surface, Gns, 1.0, 0.0, 1.0, area );

  return boost::make_shared<DenseMatrix>(*Pns - (*Gns * Gss->inverse() * *Pss));
}

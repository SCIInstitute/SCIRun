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

   Author:              Moritz Dannhauer
   Last Modification:   July 31 2012
*/


#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/BuildTDCSMatrix.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Math/MiscMath.h>

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Tensor.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;

class TDCSMatrixBuilder
{
public:
  TDCSMatrixBuilder() :
  electrodes_(1)
  {

  }
  SparseRowMatrixHandle getOutput();
  void initialize_mesh(FieldHandle mesh);
  bool initialize_inputs(SparseRowMatrixHandle stiff, DenseMatrixHandle ElectrodeElements, DenseMatrixHandle ElectrodeElementType, DenseMatrixHandle ElectrodeElementDefinition, DenseMatrixHandle contactimpedance);
  bool build_matrix(SparseRowMatrixHandle& output);
  bool singlethread();

private:
  VMesh *mesh_;

  std::vector<unsigned int> electrodes_;

  SCIRun::Core::Datatypes::SparseRowMatrixHandle stiffnessMatrix_, tdcs_;
  SCIRun::Core::Datatypes::DenseMatrixHandle electrodeElements_, electrodeElementType_, electrodeElementDefinition_, contactImpedanceInformation_;
  unsigned int electrodeElementsRows_, electrodeElementTypeRows_, electrodeElementDefinitionRows_;
  unsigned int electrodeElementsCols_, electrodeElementTypeCols_, electrodeElementDefinitionCols_;
  unsigned int mesh_nrnodes_, number_electrodes_;

};


void TDCSMatrixBuilder::initialize_mesh(FieldHandle mesh)
{
  mesh_ = mesh->vmesh();
  mesh_nrnodes_=static_cast<unsigned int>(mesh_->num_nodes());
  number_electrodes_=0;
}


bool  TDCSMatrixBuilder::singlethread()  //single threaded implementation to compute TDCS matrix for point, triangle and tetrahedral electrodes
{
///  //the stiffness-matrix updates are written into SparseRowMatrixFromMap::Values additionalData
///  //the additional matrices B,C are written into a efficient tree structure (SparseRowMatrixFromMap, thanks to Dan White) and later on added to the modified stiffness matrix to the resulting "tdcs_" - matrix
///  //the final tdcs output matrix takes the original input stiffness and overwrites it with additionalData
  size_type m = static_cast<size_type>(mesh_nrnodes_);
  size_type n = static_cast<size_type>(mesh_nrnodes_);
  SparseRowMatrixFromMap::Values additionalData;

  unsigned int p1=0,p2=0,p3=0,p4=0;
  Point pos; VMesh::Elem::index_type idx; double detJ=0.0;
  double volume=0.0, volume1_4=0.0, volume1_10=0.0, volume1_20=0.0, elc=0.0, surface_impedance=0.0, v_14_imp=0.0, v_110_imp=0.0, v_120_imp=0.0;
  double x1=0.0,x2=0.0,x3=0.0,x4=0.0,y1=0.0,y2=0.0,y3=0.0,y4=0.0,z1=0.0,z2=0.0,z3=0.0,z4=0.0,tmp=0.0,tmp1=0.0,tmp2=0.0,triangle_area=0.0;
///
  for(index_type i = 0; i<electrodeElementDefinitionRows_; i++)
  {
    if((*electrodeElementType_)(i,0)==1) //point electrodes
    {
      surface_impedance=(*contactImpedanceInformation_)(i,0);
      if (surface_impedance<=0){
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE("Contact surface impedance is negative or zero !");

      }
      tmp2=1.0/surface_impedance;
      tmp1=tmp2/2.0;
      p1=static_cast<unsigned int>((*electrodeElementDefinition_)(i,0));
      tmp=((*stiffnessMatrix_).coeff(p1,p1));
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");
      }

//     if (tmp==0)
//      {
//       std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p1 << "," << p1 << ")" << std::endl;
//       algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
//      }

      tmp+=tmp1; additionalData[p1][p1]+=tmp;
      elc=((*electrodeElements_)(i,0))+mesh_nrnodes_;
      SparseRowMatrixFromMap::Row& rowElc = additionalData[elc];
      rowElc[p1] -= tmp1;
      additionalData[p1][elc] -= tmp1;
      rowElc[elc] += tmp2*0.5;
    }
   else
   if(electrodeElementType_->coeff(i,0)==2) //electrode made of triangles
   {
     // compute triangle surface area
     mesh_->get_point(pos, VMesh::Node::index_type(electrodeElementDefinition_->coeff(i,0)));
     x1=pos.x(); y1=pos.y(); z1=pos.z();

     mesh_->get_point(pos, VMesh::Node::index_type(electrodeElementDefinition_->coeff(i,1)));
     x2=pos.x(); y2=pos.y(); z2=pos.z();

     mesh_->get_point(pos, VMesh::Node::index_type(electrodeElementDefinition_->coeff(i,2)));
     x3=pos.x(); y3=pos.y(); z3=pos.z();

     //compute triangle area in 3D
     tmp =y1*z2+z1*y3+y2*z3-z2*y3-z1*y2-y1*z3;
     tmp1=z1*x2+x1*z3+z2*x3-x2*z3-x1*z2-z1*x3;
     tmp2=x1*y2+y1*x3+x2*y3-y2*x3-y1*x2-x1*y3;

     triangle_area=0.5 * sqrt(tmp*tmp+tmp1*tmp1+tmp2*tmp2);
///
     if(triangle_area<=0) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" Triangle area should be positive! ");

     }

     surface_impedance=contactImpedanceInformation_->coeff(i,0);
     if (surface_impedance<=0)
     {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE("Contact surface impedance is negative or zeros !");

     }
     tmp1 = (2.0*triangle_area) / surface_impedance;

     p1=static_cast<unsigned int>(electrodeElementDefinition_->coeff(i,0));
     p2=static_cast<unsigned int>(electrodeElementDefinition_->coeff(i,1));
     p3=static_cast<unsigned int>(electrodeElementDefinition_->coeff(i,2));
     tmp=tmp1/6.0; //prepare value for B and Bt

     elc=electrodeElements_->coeff(i,0)+mesh_nrnodes_;
     SparseRowMatrixFromMap::Row& rowElc = additionalData[elc];
     //Bt
     rowElc[p1] -= tmp;
     rowElc[p2] -= tmp;
     rowElc[p3] -= tmp;

     //B
     additionalData[p1][elc] = rowElc[p1];
     additionalData[p2][elc] = rowElc[p2];
     additionalData[p3][elc] = rowElc[p3];
///
     tmp=tmp1/12.0;
     tmp2=stiffnessMatrix_->coeff(p1,p1);
     if(IsNan(tmp2) || !IsFinite(tmp2) || IsInfinite(tmp2)) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value ");

     }
     if (tmp2==0) {
 //     std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p1 << "," << p1 << ")" << std::endl;
 //     algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
     }
     tmp2+=tmp; additionalData[p1][p1]=tmp2;
///
     tmp2=stiffnessMatrix_->coeff(p2,p2);
     if(IsNan(tmp2) || !IsFinite(tmp2) || IsInfinite(tmp2)) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

     }
     if (tmp2==0) {
   //   std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p2 << "," << p2 << ")" << std::endl;
   //   algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
     }
     tmp2+=tmp; additionalData[p2][p2]=tmp2;

     tmp2=stiffnessMatrix_->coeff(p3,p3);
     if(IsNan(tmp2) || !IsFinite(tmp2) || IsInfinite(tmp2)) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

     }
     if (tmp2==0) {
   //   std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p3 << "," << p3 << ")" << std::endl;
   //   algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
     }
     tmp2+=tmp; additionalData[p3][p3]=tmp2;

     tmp=tmp1/24.0;
     tmp2=stiffnessMatrix_->coeff(p1,p2);
     if(IsNan(tmp2) || !IsFinite(tmp2) || IsInfinite(tmp2)) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

     }
     if (tmp2==0) {
 //     std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p1 << "," << p2 << ")" << std::endl;
 //     algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
     }
     tmp2+=tmp; additionalData[p1][p2]=tmp2;

     tmp2=stiffnessMatrix_->coeff(p1,p3);
     if(IsNan(tmp2) || !IsFinite(tmp2) || IsInfinite(tmp2)) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

     }
     if (tmp2==0) {
//      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p1 << "," << p3 << ")" << std::endl;
//      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
     }
     tmp2+=tmp; additionalData[p1][p3]=tmp2;

     tmp2=stiffnessMatrix_->coeff(p2,p1);
     if(IsNan(tmp2) || !IsFinite(tmp2) || IsInfinite(tmp2)) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

     }
     if (tmp2==0) {
 //     std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p2 << "," << p1 << ")" << std::endl;
 //     algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
     }
     tmp2+=tmp; additionalData[p2][p1]=tmp2;

     tmp2=stiffnessMatrix_->coeff(p2,p3);
     if(IsNan(tmp2) || !IsFinite(tmp2) || IsInfinite(tmp2)) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

     }
     if (tmp2==0) {
 //     std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p2 << "," << p3 << ")" << std::endl;
 //     algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
     }
     tmp2+=tmp; additionalData[p2][p3]=tmp2;

     tmp2=stiffnessMatrix_->coeff(p3,p1);
     if(IsNan(tmp2) || !IsFinite(tmp2) || IsInfinite(tmp2)) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

     }

     if (tmp2==0) {
//      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p3 << "," << p1 << ")" << std::endl;
//      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
     }
     tmp2+=tmp; additionalData[p3][p1]=tmp2;

     tmp2=stiffnessMatrix_->coeff(p3,p2);
     if(IsNan(tmp2) || !IsFinite(tmp2) || IsInfinite(tmp2)) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

     }
     if (tmp2==0) {
//      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p3 << "," << p2 << ")" << std::endl;
//      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
     }
     tmp2+=tmp; additionalData[p3][p2]=tmp2;

     additionalData[elc][elc] += 0.5*tmp1;
   } else
    if(electrodeElementType_->coeff(i,0)==3)  //electrode made of tetrahedral elements
    {
      mesh_->get_point(pos, VMesh::Node::index_type(electrodeElementDefinition_->coeff(i,0)));
      x1=pos.x(); y1=pos.y(); z1=pos.z();

      mesh_->get_point(pos, VMesh::Node::index_type(electrodeElementDefinition_->coeff(i,1)));
      x2=pos.x(); y2=pos.y(); z2=pos.z();

      mesh_->get_point(pos, VMesh::Node::index_type(electrodeElementDefinition_->coeff(i,2)));
      x3=pos.x(); y3=pos.y(); z3=pos.z();

      mesh_->get_point(pos, VMesh::Node::index_type(electrodeElementDefinition_->coeff(i,3)));
      x4=pos.x(); y4=pos.y(); z4=pos.z();

      //  compute determinant of jacobian which is needed for volume of tet
      detJ=(x3*(y2*z1-y1*z2)+ x2*(y1*z3-y3*z1) - x1*(y2*z3-y3*z2))+(-x4*(y2*z1-y1*z2)-x2* ( y1*z4 - y4*z1)+x1* ( y2*z4 - y4*z2))-(-x4*(y3*z1-y1*z3)-x3*(y1*z4-y4*z1)+x1*( y3*z4-y4*z3))+(-x4* (y3*z2-y2*z3)-x3* ( y2*z4-y4*z2)+x2* (y3*z4-y4*z3));

      if (detJ<=0)
      {
        tdcs_ = SparseRowMatrixFromMap::appendToSparseMatrix(m+number_electrodes_, n+number_electrodes_, *stiffnessMatrix_, additionalData);
        THROW_ALGORITHM_INPUT_ERROR_SIMPLE("Mesh has elements with negative/zero jacobians, check the order of the nodes that define an element");

      }
      volume=1.0/6.0*detJ; volume1_4 =volume/4.0; volume1_10=volume/10.0; volume1_20=volume/20.0;

      surface_impedance=contactImpedanceInformation_->coeff(i,0);
      if (surface_impedance<=0)
      {
        tdcs_ = SparseRowMatrixFromMap::appendToSparseMatrix(m+number_electrodes_, n+number_electrodes_, *stiffnessMatrix_, additionalData);
        THROW_ALGORITHM_INPUT_ERROR_SIMPLE("Contact surface impedance is negative or zeros !");

      }

      p1=static_cast<unsigned int>(electrodeElementDefinition_->coeff(i,0));
      p2=static_cast<unsigned int>(electrodeElementDefinition_->coeff(i,1));
      p3=static_cast<unsigned int>(electrodeElementDefinition_->coeff(i,2));
      p4=static_cast<unsigned int>(electrodeElementDefinition_->coeff(i,3));
      elc=electrodeElements_->coeff(i,0)+mesh_nrnodes_;
      v_14_imp=static_cast<double>(-volume1_4/surface_impedance);
      v_110_imp=static_cast<double>(volume1_10/surface_impedance);
      v_120_imp=static_cast<double>(volume1_20/surface_impedance);

      // Bt
      SparseRowMatrixFromMap::Row& rowElc = additionalData[elc];
      rowElc[p1] += v_14_imp;
      rowElc[p2] += v_14_imp;
      rowElc[p3] += v_14_imp;
      rowElc[p4] += v_14_imp;
      // B
      additionalData[p1][elc] = rowElc[p1];
      additionalData[p2][elc] = rowElc[p2];
      additionalData[p3][elc] = rowElc[p3];
      additionalData[p4][elc] = rowElc[p4];

      //A
      tmp=stiffnessMatrix_->coeff(p1,p1);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }
      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p1 << "," << p1 << ")" << std::endl;
//       algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_110_imp; additionalData[p1][p1]=tmp;

      tmp=stiffnessMatrix_->coeff(p2,p2);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }

      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p2 << "," << p2 << ")" << std::endl;
 //      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_110_imp; additionalData[p2][p2]=tmp;

      tmp=stiffnessMatrix_->coeff(p3,p3);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }

      if (tmp2==0) {
  //     std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p3 << "," << p3 << ")" << std::endl;
  //     algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_110_imp; additionalData[p3][p3]=tmp;

      tmp=stiffnessMatrix_->coeff(p4,p4);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");
      }

      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p4 << "," << p4 << ")" << std::endl;
 //      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_110_imp; additionalData[p4][p4]=tmp;

      tmp=stiffnessMatrix_->coeff(p1,p2);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }

      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p1 << "," << p2 << ")" << std::endl;
 //      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p1][p2]=tmp;

      tmp=stiffnessMatrix_->coeff(p2,p1);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }
      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p2 << "," << p1 << ")" << std::endl;
//       algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p2][p1]=tmp;

      tmp=stiffnessMatrix_->coeff(p1,p3);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }

      if (tmp2==0) {
//       std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p1 << "," << p3 << ")" << std::endl;
//       algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p1][p3]=tmp;
      tmp=stiffnessMatrix_->coeff(p3,p1);

      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }

      if (tmp2==0) {
//       std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p3 << "," << p1 << ")" << std::endl;
//       algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p3][p1]=tmp;

      tmp=stiffnessMatrix_->coeff(p1,p4);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }

      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p1 << "," << p4 << ")" << std::endl;
 //      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p1][p4]=tmp;

      tmp=stiffnessMatrix_->coeff(p4,p1);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }
      if (tmp2==0) {
//       std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p4 << "," << p1 << ")" << std::endl;
//       algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p4][p1]=tmp;

      tmp=stiffnessMatrix_->coeff(p2,p3);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }
      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p2 << "," << p3 << ")" << std::endl;
 //      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p2][p3]=tmp;

      tmp=stiffnessMatrix_->coeff(p3,p2);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }
      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p3 << "," << p2 << ")" << std::endl;
//       algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p3][p2]=tmp;

      tmp=stiffnessMatrix_->coeff(p2,p4);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }
      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p2 << "," << p4 << ")" << std::endl;
 //      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p2][p4]=tmp;

      tmp=stiffnessMatrix_->coeff(p4,p2);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }
      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p4 << "," << p2 << ")" << std::endl;
 //      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p4][p2]=tmp;

      tmp=stiffnessMatrix_->coeff(p3,p4);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");

      }
      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p3 << "," << p4 << ")" << std::endl;
 //      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p3][p4]=tmp;

      tmp=stiffnessMatrix_->coeff(p4,p3);
      if(IsNan(tmp) || !IsFinite(tmp) || IsInfinite(tmp)) {
       THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" No valid value in stiffnessmatrix !");
      }
      if (tmp2==0) {
 //      std::cout<<"Unexpected zero entry in stiffness matrix at ("<< p4 << "," << p3 << ")" << std::endl;
 //      algo_->remark("At least one entry in the stiffness matrix for an element (see console output) is zero. Please review the used units (conductivity, mesh location) and scale them up.");
      }
      tmp+=v_120_imp; additionalData[p4][p3]=tmp;

      //C
      rowElc[elc] += volume/surface_impedance;
    } else
   {
    tdcs_ = SparseRowMatrixFromMap::appendToSparseMatrix(m+number_electrodes_, n+number_electrodes_, *stiffnessMatrix_, additionalData);
    THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" This Electrode-type is not implemented, a electrode only consist of points(1), triangles(2) and tetrahedral elements(3).");
  }
    }

  tdcs_ = SparseRowMatrixFromMap::appendToSparseMatrix(m+number_electrodes_, n+number_electrodes_, *stiffnessMatrix_, additionalData);

  return true;
}

SparseRowMatrixHandle TDCSMatrixBuilder::getOutput()
{
  return tdcs_;
}

bool TDCSMatrixBuilder::initialize_inputs(SparseRowMatrixHandle stiff, DenseMatrixHandle ElectrodeElements,
DenseMatrixHandle ElectrodeElementType, DenseMatrixHandle ElectrodeElementDefinition, DenseMatrixHandle contactimpedance)
{
  electrodeElements_=(ElectrodeElements);
  electrodeElementType_=(ElectrodeElementType);
  electrodeElementDefinition_=(ElectrodeElementDefinition);

  //check matrices dimensions
  electrodeElementsRows_=static_cast<unsigned int>(electrodeElements_->nrows());
  electrodeElementTypeRows_=static_cast<unsigned int>(electrodeElementType_->nrows());
  electrodeElementDefinitionRows_=static_cast<unsigned int>(electrodeElementDefinition_->nrows());
  electrodeElementsCols_=static_cast<unsigned int>(electrodeElements_->ncols());
  electrodeElementTypeCols_=static_cast<unsigned int>(electrodeElementType_->ncols());
  electrodeElementDefinitionCols_=static_cast<unsigned int>(electrodeElementDefinition_->ncols());

  if( !((electrodeElementsRows_==electrodeElementTypeRows_) && (electrodeElementTypeRows_==electrodeElementDefinitionRows_)) ) {
    THROW_ALGORITHM_INPUT_ERROR_SIMPLE("Number of Matrix-rows of Matrices for Electrode-Definition should be the same!");
  }

  if( !((electrodeElementsCols_==1) && (electrodeElementTypeCols_==1) && (electrodeElementDefinitionCols_>=1 && electrodeElementDefinitionCols_<=4) ) ) {
    THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" Number of Matrix-columns of Matrices in Electrode-Definition should be: ElectrodeElementType=1, ElectrodeElementDefinition=1,ElectrodeElementDefinition=4 !");
  }

  if (electrodeElementsRows_>mesh_nrnodes_) {
    THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" Number of Electrode-Definition Nodes can not excced number of input mesh (since it refers to that) !");
  }

  if ( electrodeElementsRows_==0 ) {
    THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" Number of Electrode-Definition Nodes = 0 !");
  }

  //check matrices content
  electrodes_.push_back(static_cast<unsigned int>((*electrodeElements_)(0,0)));
  number_electrodes_=1; bool numbering_ok=false;
  for (unsigned int i=0;i<electrodeElementsRows_;i++)
  {
    bool found = false;
    if (static_cast<unsigned int>((*electrodeElements_)(0,0))==0)  {
      numbering_ok=true;
    }

    unsigned int tmp=static_cast<unsigned int>((*electrodeElements_)(i,0));
    if( tmp>=mesh_nrnodes_ ) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" Specified ElectrodeElement node out of range (0..#inputmeshnodes-1) !");
    }

    unsigned int tmp1=static_cast<unsigned int>((*electrodeElementType_)(i,0));
    if ( !(tmp1>=1 && tmp1<=3) ) {
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" Specified ElectrodeElementType out of range, allowed range: 1 (point), 2 (triangle), 3 (tetrahedra) !");
    }

    for(unsigned int j=0;j<electrodeElementDefinitionCols_;j++)
    {
      unsigned int tmp2=static_cast<unsigned int>((*electrodeElementDefinition_)(i,j));
      if ( tmp2>=mesh_nrnodes_ ) {
        THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" Specified ElectrodeElementDefinition is out of range (> mesh nodes-1) - allowed range is (allowed range: 0..#meshnodes-1) !");
      }
    }

    for (unsigned int j=0;j<electrodes_.size();j++)
    {
      if (electrodes_[j]==tmp)
      {
        found = true;
        break;
      }
    }

    if (! found)
    {
      electrodes_.push_back((*electrodeElements_)(i,0));
      number_electrodes_++;
    }
  }

  if (!numbering_ok)
  {
   THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" The electrode numbering should should start at 0 (allowed range: 0..#meshnodes-1) !");
  }

   // get surface impedance
  if(contactimpedance)
  {
    contactImpedanceInformation_ = contactimpedance;
    if(static_cast<unsigned int>(contactImpedanceInformation_->nrows())!=electrodeElementsRows_) {
     THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" Contact surface impedance vector and electrode definition does not fit !");
    }
  }
  else
  {
    contactImpedanceInformation_.reset(new DenseMatrix(electrodeElementsRows_,1));
    for(unsigned int i=0;i<electrodeElementsRows_;i++)
      (*contactImpedanceInformation_)(i,0)=1;
  }

  if (  !( (contactImpedanceInformation_->nrows()==electrodeElementsRows_) && (contactImpedanceInformation_->ncols()==1))  ) {
    THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" ContactImpedanceMatrix should have matrix dimensions: #electrodeselementsx1 !");
  }

  stiffnessMatrix_ = stiff;
  if ( !( (stiffnessMatrix_->nrows()>0) && (stiffnessMatrix_->nrows()==stiffnessMatrix_->ncols()))) {
    THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" StiffnessMatrix should be square and non-empty !");
  }
  if ( !( stiffnessMatrix_->nrows()==mesh_nrnodes_  ) ) {
    THROW_ALGORITHM_INPUT_ERROR_SIMPLE(" StiffnessMatrix should have the same number of nodes as inputmesh !");
  }

  stiffnessMatrix_->makeCompressed();

  return true;
}


bool TDCSMatrixBuilder::build_matrix(SparseRowMatrixHandle& output)
{
  return singlethread();
}

bool BuildTDCSMatrixAlgo::run(SparseRowMatrixHandle stiff, FieldHandle mesh, DenseMatrixHandle ElectrodeElements, DenseMatrixHandle ElectrodeElementType, DenseMatrixHandle
ElectrodeElementDefinition, DenseMatrixHandle contactimpedance, SparseRowMatrixHandle& output) const
{

 ScopedAlgorithmStatusReporter asc(this, "Name");

 if (! mesh)
 {
   error(" Without a mesh there is nothing to do !");

   return (false);
 }

 if (! ElectrodeElements) //get electrode definition
 {
   error("ElectrodeElements object not available....");

   return false;
 }

 if (! ElectrodeElementType) //get electrode definition
 {
   error("ElectrodeElementType object not available....");

   return false;
 }

 if (! ElectrodeElementDefinition) //get electrode definition
 {
   error("ElectrodeElementDefinition object not available....");

   return false;
 }

 TDCSMatrixBuilder builder;
///
 builder.initialize_mesh(mesh); //set mesh
///
 if (! builder.initialize_inputs(stiff, ElectrodeElements, ElectrodeElementType, ElectrodeElementDefinition, contactimpedance) ) // set other inputs
 {

   return false;
 }
///
 if (! builder.build_matrix(output))
 {

   return false;
 }
///

 SparseRowMatrixHandle tdcs=builder.getOutput();
 output = tdcs;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

  bool found_reference_node=false;
  for(index_type i = 0; i<tdcs->nrows()-1; i++) //find reference node
  {
    index_type ps = (*tdcs).get_row(i);
    index_type pe = (*tdcs).get_row(i+1);
    if((pe-ps)==1)
       {
        if (static_cast<double>((*tdcs).get_value(ps))==1.0)
        found_reference_node=true;
 	break;
       }
  }

  if(!found_reference_node)
      remark("The TDCS output matrix is not referenced yet !!! Please set the Potential of at least one node to 0 ! You can do that by setting one row and the corresponding column explicitely to 0 except for the diagonal element which should be 1 !");

#endif
 return (true);
}

AlgorithmInputName BuildTDCSMatrixAlgo::FEM_Stiffness_Matrix("FEM_Stiffness_Matrix");
AlgorithmInputName BuildTDCSMatrixAlgo::FEM_Mesh("FEM_Mesh");
AlgorithmInputName BuildTDCSMatrixAlgo::Electrode_Element("Electrode_Element");
AlgorithmInputName BuildTDCSMatrixAlgo::Electrode_Element_Type("Electrode_Element_Type");
AlgorithmInputName BuildTDCSMatrixAlgo::Electrode_Element_Definition("Electrode_Element_Definition");
AlgorithmInputName BuildTDCSMatrixAlgo::Contact_Impedance("Contact_Impedance");
AlgorithmOutputName BuildTDCSMatrixAlgo::TDCSMatrix("TDCSMatrix");

//(FEM_Stiffness_Matrix,Stiffness)(FEM_Mesh,Mesh)(Eletrode_Element,ElectrodeElements)(Electrode_Element_Type,ElectrodeElementType)(Electrode_Element_Definition,ElectrodeElementDefinition)(Contact_Impedance,ContactImpedance))
AlgorithmOutput BuildTDCSMatrixAlgo::run(const AlgorithmInput & input) const
{
  auto a = input.get<SparseRowMatrix>(FEM_Stiffness_Matrix);
  auto b = input.get<Field>(FEM_Mesh);
  auto c = input.get<DenseMatrix>(Electrode_Element);
  auto d = input.get<DenseMatrix>(Electrode_Element_Type);
  auto e = input.get<DenseMatrix>(Electrode_Element_Definition);
  auto f = input.get<DenseMatrix>(Contact_Impedance);

  SparseRowMatrixHandle tdcs;
  if (!run(a,b,c,d,e,f,tdcs))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[TDCSMatrix] = tdcs;
  return output;
}

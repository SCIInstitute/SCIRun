/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
   
   Author            : Moritz Dannhauer
   Last modification : September 15 2015
   TODO              : implement in parallel execution, transient GUI variables
*/
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>
#include <Core/Algorithms/Field/RefineCleaverMeshAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Field/InterfaceWithCleaverAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Utils/StringUtil.h>
#include <boost/scoped_ptr.hpp>
#include <Core/Logging/Log.h>
#include <Core/Math/MiscMath.h>
#include <vector>
#include <iostream>
#include <iterator>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;

ALGORITHM_PARAMETER_DEF(Fields, RefineCleaverMeshIsoValue);
ALGORITHM_PARAMETER_DEF(Fields, RefineCleaverMeshEdgeLength);
ALGORITHM_PARAMETER_DEF(Fields, RefineCleaverMeshVolume);
ALGORITHM_PARAMETER_DEF(Fields, RefineCleaverMeshDihedralAngleSmaller);
ALGORITHM_PARAMETER_DEF(Fields, RefineCleaverMeshDihedralAngleBigger);
ALGORITHM_PARAMETER_DEF(Fields, RefineCleaverMeshDoNoSplitSurfaceTets);
ALGORITHM_PARAMETER_DEF(Fields, RefineCleaverMeshRadioButtons);

RefineCleaverMeshAlgorithm::RefineCleaverMeshAlgorithm()
{
  using namespace Parameters;
  
  addParameter(RefineCleaverMeshIsoValue, 0.0);
  addParameter(RefineCleaverMeshEdgeLength, 0.0);
  addParameter(RefineCleaverMeshVolume, 0.0);
  addParameter(RefineCleaverMeshDihedralAngleSmaller, 180.0);
  addParameter(RefineCleaverMeshDihedralAngleBigger, 0.0);
  addParameter(RefineCleaverMeshRadioButtons, 0);
  addParameter(RefineCleaverMeshDoNoSplitSurfaceTets, false);
}

AlgorithmOutput RefineCleaverMeshAlgorithm::run_generic(const AlgorithmInput& input) const 
{ 
  auto inputfield = input.get<Field>(Variables::InputField);
  auto inputmatrix = input.get<DenseMatrix>(Variables::InputMatrix);
  FieldHandle outputField;

  if (!runImpl(inputfield, inputmatrix, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");
    
  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  
  return output;
}

std::vector<bool> RefineCleaverMeshAlgorithm::SelectMeshElements(FieldHandle input, DenseMatrixHandle matrix, int choose_refinement_option, int& count) const
{
 /// if a matrix second module input is provided used for element selection (that get split) and ignore GUI settings
 std::vector<bool> result;
 using namespace Parameters;
 FieldHandle output;
 
 VMesh* input_vmesh = input->vmesh(); 
 VField* input_vfld = input->vfield(); 
 input_vmesh->synchronize(Mesh::NODES_E);
 if(matrix)
 {
  std::cout << "drin: " << std::endl;
  if(input_vfld->num_values() != matrix->nrows() || input_vfld->num_values() != matrix->ncols())
  {
   error("Dimensions of provided MATRIX input (second module input port) does not fit provided FIELD (first module input port).");
   return result; 
  }
  if (input_vfld->num_values() == matrix->nrows())
  {
   error("Number of columns of provided MATRIX input (second module input port) needs to fit number of FIELD data (first module input port).");
   return result; 
  }
  result.resize(input_vfld->num_values());
  /// use matrix to do the element selection
  if( input_vfld->num_values() == matrix->nrows() )
  {
    for (long i=0;i<matrix->nrows();i++)
    {
      if((*matrix)(i,0)!=0)
       {
         result[i]=true;
	 count++;
       } else
         result[i]=false;
    }
  } else
  if ( input_vfld->num_values() == matrix->ncols() )
  {
    for (long i=0;i<matrix->ncols();i++)
    {
      if((*matrix)(0,i)!=0)
       {
         result[i]=true;
	 count++;
       } else
         result[i]=false;
    }
  } else
  {
    error("Dimensions of provided MATRIX input (second module input port) does not fit provided FIELD (first module input port).");
    return result; 
  }
  return result;
 }
 
  switch (choose_refinement_option)
  {    
    case 0: /// this is the isovalue selection criteria
    {
      double value=get(RefineCleaverMeshIsoValue).toDouble();
      result.resize(input_vmesh->num_elems());
      for (VMesh::Elem::index_type idx=0; idx<input_vmesh->num_elems(); idx++)
      {
        double tmp;
        input_vfld->get_value(tmp,idx);
	if (tmp==value)
	{
	 result[(long)idx]=true; 
	 count++;
	} else
	{
	 result[(long)idx]=false; 
	}
      }
     break;
    }
    case 1: /// this is the edge selection criteria
    {
     double value=get(RefineCleaverMeshEdgeLength).toDouble();
     result.resize(input_vmesh->num_elems());
     std::vector<double> maxi(6);
     for (VMesh::Elem::index_type idx=0; idx<input_vmesh->num_elems(); idx++)
     {
      VMesh::Node::array_type onodes(4); 
      input_vmesh->get_nodes(onodes, idx);
      Point p1,p2,p3,p4;
      input_vmesh->get_center(p1,onodes[0]);
      input_vmesh->get_center(p2,onodes[1]);
      input_vmesh->get_center(p3,onodes[2]);
      input_vmesh->get_center(p4,onodes[3]);
      double x1=p1.x(),y1=p1.y(),z1=p1.z(),
             x2=p2.x(),y2=p2.y(),z2=p2.z(),
             x3=p3.x(),y3=p3.y(),z3=p3.z(),
             x4=p4.x(),y4=p4.y(),z4=p4.z();
      double e12=sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2)),
             e14=sqrt((x1-x4)*(x1-x4)+(y1-y4)*(y1-y4)+(z1-z4)*(z1-z4)),
	     e13=sqrt((x1-x3)*(x1-x3)+(y1-y3)*(y1-y3)+(z1-z3)*(z1-z3)),
	     e34=sqrt((x3-x4)*(x3-x4)+(y3-y4)*(y3-y4)+(z3-z4)*(z3-z4)),
	     e23=sqrt((x2-x3)*(x2-x3)+(y2-y3)*(y2-y3)+(z2-z3)*(z2-z3)),
	     e24=sqrt((x2-x4)*(x2-x4)+(y2-y4)*(y2-y4)+(z2-z4)*(z2-z4));
      maxi[0]=(e12);maxi[1]=(e13);maxi[2]=(e14);maxi[3]=(e34);maxi[4]=(e23); maxi[5]=(e24);
      std::vector<double>::iterator pos = max_element (maxi.begin(), maxi.end()); // does it work?
      if (*pos > value)
      {
       result[(long)idx]=true; 
       count++;
      } else
      {
       result[(long)idx]=false; 
      }   
     }
     break;
    }
    case 2: /// this is the volume selection criteria
    {
     double volume_bound=get(RefineCleaverMeshVolume).toDouble();
     result.resize(input_vmesh->num_elems());
     for (VMesh::Elem::index_type idx=0; idx<input_vmesh->num_elems(); idx++)
     {
      VMesh::Node::array_type onodes(4); 
      input_vmesh->get_nodes(onodes, idx);
      Point p1,p2,p3,p4;
      input_vmesh->get_center(p1,onodes[0]);
      input_vmesh->get_center(p2,onodes[1]);
      input_vmesh->get_center(p3,onodes[2]);
      input_vmesh->get_center(p4,onodes[3]);
      double x1=p1.x(),y1=p1.y(),z1=p1.z(),
             x2=p2.x(),y2=p2.y(),z2=p2.z(),
             x3=p3.x(),y3=p3.y(),z3=p3.z(),
             x4=p4.x(),y4=p4.y(),z4=p4.z();
      double tet_volume=1/6*((x3*(y2*z1-y1*z2)+ x2*(y1*z3-y3*z1) - x1*(y2*z3-y3*z2))+
           (-x4*(y2*z1-y1*z2)-x2* ( y1*z4 - y4*z1)+x1* ( y2*z4 - y4*z2))-
	   (-x4*(y3*z1-y1*z3)-x3*(y1*z4-y4*z1)+x1*( y3*z4-y4*z3))+(-x4* (y3*z2-y2*z3)-x3* 
	   ( y2*z4-y4*z2)+x2*(y3*z4-y4*z3)));
	   
      if (tet_volume<0)
      {
       error(" The volume of at least one mesh element is zero. The definition of the elements might be different from a typical cleaver mesh.");
       return result; 
      }
      if (tet_volume<volume_bound)
      {
	result[(long)idx]=true;  
	count++;
      } else
      {
	result[(long)idx]=false;  
      }
     }
     break;
    }
    case 3:
    {
     std::vector<Point> points(3);
     VMesh::Node::array_type nodes;
     VMesh::Face::array_type faces;
     double min=std::numeric_limits<double>::max(),max=std::numeric_limits<double>::min();
     for (VMesh::Elem::index_type idx=0; idx<input_vmesh->num_elems(); idx++)
     {
      input_vmesh->get_faces(faces, idx);
      if (faces.size()!=4)
      {
       error(" The chosen refinement criteria is not valid. This message should not appear!!!");
       std::vector<bool> tmp;
       return tmp; 
       break;
      }
      double min_bound=get(RefineCleaverMeshDihedralAngleBigger).toDouble();
      double max_bound=get(RefineCleaverMeshDihedralAngleSmaller).toDouble();
      for (int j=0; j<4; j++) 
      {
       min=std::numeric_limits<double>::max();max=std::numeric_limits<double>::min();
       for (int k=j+1; k<4; k++) 
       {       
        input_vmesh->get_nodes(nodes, faces[j]);
        input_vmesh->get_centers(points, nodes);
        Vector normal1 = Cross(points[1]-points[0], points[2]-points[0]);
	normal1.safe_normalize();
	input_vmesh->get_nodes(nodes, faces[k]);
        input_vmesh->get_centers(points, nodes);
        Vector normal2 = Cross(points[1]-points[0], points[2]-points[0]);
	normal2.safe_normalize();
	double dot_product=Dot(normal1, normal2);
        if (dot_product < -1) 
	{
          dot_product = -1;
        } else if (dot_product > 1) 
	{
          dot_product = 1;
        }
        double dihedral_angle = 180.0 - acos(dot_product) * 180.0 / M_PI;

        if (dihedral_angle < min)
        {
         min = dihedral_angle;
        }
        else if(dihedral_angle > max)
        {
         max = dihedral_angle;
        }     
       }
      }
      
      if (min>=min_bound && max<=max_bound)
       {
	result.push_back(true); 
	count++;
       } else
       {
	result.push_back(false);
       }
     }
     break;
    }
    default :
    {
      error(" The chosen refinement criteria is not valid. This message should not appear!!!");
      std::vector<bool> tmp;
      return tmp; 
      break;
    }
  }

 if(input_vmesh->num_elems()!=result.size())
 {
   error(" Size of selection vector is unexpected ");
   std::vector<bool> tmp; 
   return tmp; 
 }

 bool MeshDoNoSplitSurfaceTets = get(RefineCleaverMeshDoNoSplitSurfaceTets).toBool();

 if (MeshDoNoSplitSurfaceTets)
 {  
  std::cout << "oh?: " << std::endl;
  GetFieldBoundaryAlgo getfieldbound_algo;
  MatrixHandle mapping;
  getfieldbound_algo.run(input,output,mapping);
  std::cout << "oh!: " << std::endl;
  if (mapping)
  {
   SparseRowMatrixHandle tmp = boost::make_shared<SparseRowMatrix>(*matrix_cast::as_sparse(mapping));

   for (index_type idx=0; idx<tmp->ncols(); idx++)
   {
    auto col = tmp->getColumn(idx);
    if (col.nonZeros()>0)
    {
     result[idx]=false;
    }
   }
  
  } else
  {
   error(" The mapping matrix (not splitting surface boundary elements) from GetFieldBoundary is empty.");
   std::vector<bool> tmp; 
   return tmp; 
  }

 }

 return result;
}


bool RefineCleaverMeshAlgorithm::ComputeEdgeMidPoint(int pos, Point p1, Point p2, Point p3, Point p4, Point& output) const
{
 switch (pos)
 {
  default:
  {
   return false;
   break;
  }
  case 0: // 1-2
  {
   output=Point((p1.x()-p2.x())/2+p2.x(),(p1.y()-p2.y())/2+p2.y(),(p1.z()-p2.z())/2+p2.z());
   
   break;
  }
  case 1: // 1-3
  {
   output=Point((p1.x()-p3.x())/2+p3.x(),(p1.y()-p3.y())/2+p3.y(),(p1.z()-p3.z())/2+p3.z());
   break;
  } 
  case 2: // 1-4
  {
   output=Point((p1.x()-p4.x())/2+p4.x(),(p1.y()-p4.y())/2+p4.y(),(p1.z()-p4.z())/2+p4.z());
   break;
  }
  case 3: // 2-3
  {
   output=Point((p2.x()-p3.x())/2+p3.x(),(p2.y()-p3.y())/2+p3.y(),(p2.z()-p3.z())/2+p3.z());  
   break;
  }
  case 4: // 2-4
  {
   output=Point((p2.x()-p4.x())/2+p4.x(),(p2.y()-p4.y())/2+p4.y(),(p2.z()-p4.z())/2+p4.z());  
   break;
  }
  case 5: // 3-4
  {
   output=Point((p3.x()-p4.x())/2+p4.x(),(p3.y()-p4.y())/2+p4.y(),(p3.z()-p4.z())/2+p4.z());
   break;
  }
 }

 return true; 
}

std::vector<double> RefineCleaverMeshAlgorithm::getEdgeLengths(Point p1, Point p2, Point p3, Point p4) const
{
   std::vector<double> edge_lengths(6);
  
  double x1=p1.x(),y1=p1.y(),z1=p1.z(),
          x2=p2.x(),y2=p2.y(),z2=p2.z(),
          x3=p3.x(),y3=p3.y(),z3=p3.z(),
          x4=p4.x(),y4=p4.y(),z4=p4.z();
  double e12=sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2)),
          e14=sqrt((x1-x4)*(x1-x4)+(y1-y4)*(y1-y4)+(z1-z4)*(z1-z4)),
	  e13=sqrt((x1-x3)*(x1-x3)+(y1-y3)*(y1-y3)+(z1-z3)*(z1-z3)),
	  e34=sqrt((x3-x4)*(x3-x4)+(y3-y4)*(y3-y4)+(z3-z4)*(z3-z4)),
	  e23=sqrt((x2-x3)*(x2-x3)+(y2-y3)*(y2-y3)+(z2-z3)*(z2-z3)),
	  e24=sqrt((x2-x4)*(x2-x4)+(y2-y4)*(y2-y4)+(z2-z4)*(z2-z4));
   /// definition of edges is hardcoded and assumed in other functions called here, e.g. first is 1-2, second 1-3, etc.
   edge_lengths[0]=e12;edge_lengths[1]=e13;edge_lengths[2]=(e14);edge_lengths[3]=(e23);edge_lengths[4]=(e24); edge_lengths[5]=(e34);
   
  return edge_lengths;
}

std::vector<int> RefineCleaverMeshAlgorithm::getEdgeCoding(int pos) const
{
  std::vector<int> edgecoding;
  edgecoding.resize(2);
  switch (pos)
 {
  default:
  {
   error("Error in getEdgeCoding. This message should never appear. ");
   return edgecoding;
   break;
  }
  case 0: // 1-2
  {
   edgecoding[0]=1;
   edgecoding[1]=2;
   break;
  }
  case 1: // 1-3
  {
   edgecoding[0]=1;
   edgecoding[1]=3;
   break;
  } 
  case 2: // 1-4
  {
   edgecoding[0]=1;
   edgecoding[1]=4;
   break;
  }
  case 3: // 2-3
  {
   edgecoding[0]=2;
   edgecoding[1]=3; 
   break;
  }
  case 4: // 2-4
  {
   edgecoding[0]=2;
   edgecoding[1]=4; 
   break;
  }
  case 5: // 3-4
  {
   edgecoding[0]=3;
   edgecoding[1]=4; 
   break;
  }
 }
 return edgecoding;
}

bool RefineCleaverMeshAlgorithm::SplitTet(VMesh::Node::array_type cell, int edge_pos, DenseMatrixHandle two_new_tets, long& node_count) const
{
 switch (edge_pos)
 {
  default:
  {
   return false;
  }

   case 0: // 1-2
   {
    (*two_new_tets)(0,0)=cell[0];    (*two_new_tets)(0,1)=node_count; (*two_new_tets)(0,2)=cell[2]; (*two_new_tets)(0,3)=cell[3];
    (*two_new_tets)(1,0)=node_count; (*two_new_tets)(1,1)=cell[1];    (*two_new_tets)(1,2)=cell[2]; (*two_new_tets)(1,3)=cell[3];
    break;
   }
   case 1: // 1-3
   {
    (*two_new_tets)(0,0)=cell[0];     (*two_new_tets)(0,1)=cell[1]; (*two_new_tets)(0,2)=node_count; (*two_new_tets)(0,3)=cell[3];
    (*two_new_tets)(1,0)=node_count;  (*two_new_tets)(1,1)=cell[1]; (*two_new_tets)(1,2)=cell[2];     (*two_new_tets)(1,3)=cell[3];
    break;
   }
   
   case 2: // 1-4
   {
    (*two_new_tets)(0,0)=cell[0];     (*two_new_tets)(0,1)=cell[1];   (*two_new_tets)(0,2)=cell[2];  (*two_new_tets)(0,3)=node_count;
    (*two_new_tets)(1,0)=node_count;  (*two_new_tets)(1,1)=cell[1];   (*two_new_tets)(1,2)=cell[2];  (*two_new_tets)(1,3)=cell[3];
    break;
   }
   
   case 3: // 2-3
   {
    (*two_new_tets)(0,0)=cell[0]; (*two_new_tets)(0,1)=cell[1];   (*two_new_tets)(0,2)=node_count;  (*two_new_tets)(0,3)=cell[3];
    (*two_new_tets)(1,0)=cell[0]; (*two_new_tets)(1,1)=node_count;(*two_new_tets)(1,2)=cell[2];     (*two_new_tets)(1,3)=cell[3];
    break;
   }
   
   case 4: // 2-4
   {
    (*two_new_tets)(0,0)=cell[0]; (*two_new_tets)(0,1)=cell[1];     (*two_new_tets)(0,2)=cell[2];  (*two_new_tets)(0,3)=node_count;
    (*two_new_tets)(1,0)=cell[0]; (*two_new_tets)(1,1)=node_count;  (*two_new_tets)(1,2)=cell[2];  (*two_new_tets)(1,3)=cell[3];
    break;
   }
   
   case 5: // 3-4
   {
    (*two_new_tets)(0,0)=cell[0]; (*two_new_tets)(0,1)=cell[1];     (*two_new_tets)(0,2)=cell[2];    (*two_new_tets)(0,3)=node_count;
    (*two_new_tets)(1,0)=cell[0]; (*two_new_tets)(1,1)=cell[1];     (*two_new_tets)(1,2)=node_count; (*two_new_tets)(1,3)=cell[3];
    break;
   }
 }

 return true;
}

std::vector<int> RefineCleaverMeshAlgorithm::maxi(std::vector<double> input_vec) const
{/// if there are 1 or 2 maxima in input_vec the index(e) of them should be in the output.
 std::vector<int> result;
 double maximum=std::numeric_limits<double>::min();
 int ind=-1,ind2=-1,count=0;
 
 for(int i=0;i<input_vec.size();i++)
 {
   if (input_vec[i]>maximum)
   { 
    maximum=input_vec[i];
    ind=i;
   }
 }
 
 for(int i=0;i<input_vec.size();i++)
 {
   if (input_vec[i]==maximum)
     count++;
 }

 for(int i=ind+1;i<input_vec.size();i++)
 {
   if (input_vec[i]==maximum)
     ind2=i;
 }  

 result.resize(count);
 result[0]=ind;
 if (count==2)
    result[1]=ind2;
    
 return result;
}


FieldHandle RefineCleaverMeshAlgorithm::RefineMesh(FieldHandle input, std::vector<long> elems_to_split) const
{
 FieldHandle result;
 VMesh* input_vmesh = input->vmesh();
 std::vector<double> edge_lengths,tet_field_values;std::vector<int> remaining_edges(2); 
 VMesh::Node::array_type onodes(4),onodes2(4); 
 edge_lengths.resize(6);
 if(elems_to_split.size()!=input_vmesh->num_elems())
 {
   error("Length of Selection criteria vector and number of mesh elements are different.");
   return result;
 } 
 double fld_val;  
 tet_field_values.resize(input_vmesh->num_elems());
 input_vmesh->synchronize(Mesh::NODES_E);
 DenseMatrixHandle new_tets(new DenseMatrix(2*elems_to_split.size(), 4)),new_nodes(new DenseMatrix(2*elems_to_split.size(), 3)),two_new_tets(new DenseMatrix(2, 4));
 long tet_count=0,node_count=input_vmesh->num_nodes();

 for (long idx=0;idx<elems_to_split.size();idx++) // this for loop can be parallized
 {
   input_vmesh->get_nodes(onodes, (VMesh::Elem::index_type)elems_to_split[idx]);
   VMesh* input_vmesh   = input->vmesh();
   VField* input_vfield = input->vfield();
   input_vfield->get_value(fld_val,idx);
   tet_field_values[(long)idx]=fld_val;
   Point p1,p2,p3,p4;
   input_vmesh->get_center(p1,onodes[0]);
   input_vmesh->get_center(p2,onodes[1]);
   input_vmesh->get_center(p3,onodes[2]);
   input_vmesh->get_center(p4,onodes[3]);
   edge_lengths=getEdgeLengths(p1, p2, p3, p4);  
   std::vector<int> pos = maxi(edge_lengths);    
   
   switch (pos.size())
   { 
    default:
    {
     error("This type of mesh element (more then 2 equally long mesh edges) is not supported and might not belong to a cleaver mesh.");
     FieldHandle tmp;
     return tmp; 
     break;
    }
    case  1:
    {
      Point out;
      ComputeEdgeMidPoint(pos[0], p1, p2, p3, p4, out);
      SplitTet(onodes, pos[0], two_new_tets, node_count);
      (*new_nodes)(node_count  ,0)=out.x(); 
      (*new_nodes)(node_count  ,1)=out.y(); 
      (*new_nodes)(node_count++,2)=out.z();
      (*new_tets)(tet_count  ,0)=(*two_new_tets)(0,0); 
      (*new_tets)(tet_count  ,1)=(*two_new_tets)(0,1); 
      (*new_tets)(tet_count  ,2)=(*two_new_tets)(0,2); 
      (*new_tets)(tet_count++,3)=(*two_new_tets)(0,3);
      (*new_tets)(tet_count  ,0)=(*two_new_tets)(1,0); 
      (*new_tets)(tet_count  ,1)=(*two_new_tets)(1,1); 
      (*new_tets)(tet_count  ,2)=(*two_new_tets)(1,2); 
      (*new_tets)(tet_count++,3)=(*two_new_tets)(1,3);       
       break;
    }
    case 2: /// 2 edges have the same (maximum) lentgth = 4 new tets need to be created
    {
      Point out;
      ComputeEdgeMidPoint(pos[0], p1, p2, p3, p4, out);      /// first cut point
      SplitTet(onodes, pos[0], two_new_tets, node_count);
      (*new_nodes)(node_count  ,0)=out.x(); 
      (*new_nodes)(node_count  ,1)=out.y(); 
      (*new_nodes)(node_count++,2)=out.z();  
      remaining_edges=getEdgeCoding(pos[1]);
      remaining_edges[0]=onodes[remaining_edges[0]-1];
      remaining_edges[1]=onodes[remaining_edges[1]-1];
      input_vmesh->get_center(p1, (VMesh::Node::index_type)remaining_edges[0]);
      input_vmesh->get_center(p2, (VMesh::Node::index_type)remaining_edges[1]);
      Point out2((p1.x()+p2.x())/2, (p1.y()+p2.y())/2, (p1.z()+p2.z())/2);  /// second cut point
      onodes[0] =(*two_new_tets)(0,0); onodes[0] =(*two_new_tets)(0,1);  onodes[0] =(*two_new_tets)(0,2); onodes[0] =(*two_new_tets)(0,3);
      onodes2[0]=(*two_new_tets)(1,0); onodes2[0]=(*two_new_tets)(1,1);  onodes2[0]=(*two_new_tets)(1,2); onodes2[0]=(*two_new_tets)(1,3);
      SplitTet(onodes, pos[1], two_new_tets, node_count); 
      (*new_nodes)(node_count  ,0)=out2.x(); 
      (*new_nodes)(node_count  ,1)=out2.y(); 
      (*new_nodes)(node_count++,2)=out2.z();  
      
      (*new_tets)(tet_count  ,0)=(*two_new_tets)(0,0); /// new tet 1 and 2
      (*new_tets)(tet_count  ,1)=(*two_new_tets)(0,1); 
      (*new_tets)(tet_count  ,2)=(*two_new_tets)(0,2); 
      std::cout << "t1:" << (*new_tets)(tet_count  ,0) << " " << (*new_tets)(tet_count  ,1) << " " << (*new_tets)(tet_count  ,2) << " " << (*two_new_tets)(0,3) << std::endl;
      (*new_tets)(tet_count++,3)=(*two_new_tets)(0,3);
      (*new_tets)(tet_count  ,0)=(*two_new_tets)(1,0); 
      (*new_tets)(tet_count  ,1)=(*two_new_tets)(1,1); 
      (*new_tets)(tet_count  ,2)=(*two_new_tets)(1,2); 
      std::cout << "t2:" << (*new_tets)(tet_count  ,0) << " " << (*new_tets)(tet_count  ,1) << " " << (*new_tets)(tet_count  ,2) << " " << (*two_new_tets)(1,3) << std::endl;
      (*new_tets)(tet_count++,3)=(*two_new_tets)(1,3);  
      
      SplitTet(onodes2, pos[1], two_new_tets, node_count); 
      (*new_tets)(tet_count  ,0)=(*two_new_tets)(0,0); /// new tet 3 and 4
      (*new_tets)(tet_count  ,1)=(*two_new_tets)(0,1); 
      (*new_tets)(tet_count  ,2)=(*two_new_tets)(0,2); 
      std::cout << "t3:" << (*new_tets)(tet_count  ,0) << " " << (*new_tets)(tet_count  ,1) << " " << (*new_tets)(tet_count  ,2) << " " << (*two_new_tets)(0,3) << std::endl;
      (*new_tets)(tet_count++,3)=(*two_new_tets)(0,3);
      (*new_tets)(tet_count  ,0)=(*two_new_tets)(1,0); 
      (*new_tets)(tet_count  ,1)=(*two_new_tets)(1,1); 
      (*new_tets)(tet_count  ,2)=(*two_new_tets)(1,2);
      std::cout << "t4:" << (*new_tets)(tet_count  ,0) << " " << (*new_tets)(tet_count  ,1) << " " << (*new_tets)(tet_count  ,2) << " " << (*two_new_tets)(1,3) << std::endl;
      (*new_tets)(tet_count++,3)=(*two_new_tets)(1,3);  
      
      /*     
      std::vector<Point> points(4); 
      for(int i=0;i<2;i++) 
      {
       for(int j=0;j<4;j++)
        {
	  int elem_node=(*two_new_tets)(i,j);
	  if(elem_node>input_vmesh->num_nodes())
	    points[j]=out;
	     else
	     {
	      Point p0;
	      input_vmesh->get_center(p0,(VMesh::Elem::index_type)elem_node);
	      points[j]=p0;
	     }
	}
	
        edge_lengths=getEdgeLengths(points[0], points[1], points[2], points[3]); /// find the longest edge
	std::vector<int> pos2 = maxi(edge_lengths);
      }*/
      //std::cout << "c:" << count << " " << ind << std::endl;
      
      /*
      if (ind==-1)
      {
       error("unexpected error in refinemesh() - this message should not appear"); 
      } else
      if (ind==0) /// not tested
      {
        (*new_tets)(tet_count  ,0)=(*two_new_tets)(1,0); /// save the tet that will not be splitted because it does not contain the second large edge that is equal
        (*new_tets)(tet_count  ,1)=(*two_new_tets)(1,1); /// to the first one (but that has been already been split)
        (*new_tets)(tet_count  ,2)=(*two_new_tets)(1,2); 
        (*new_tets)(tet_count++,3)=(*two_new_tets)(1,3);
  	
	if((*two_new_tets)(0,0)>=input_vmesh->num_nodes())  /// get the node locations for the tet to be split
	{
	 p1=out;
	} else
   	 input_vmesh->get_center(p1,(VMesh::Node::index_type)(*two_new_tets)(0,0));

        if((*two_new_tets)(0,1)>=input_vmesh->num_nodes())  
	{
	 p2=out;
	} else
	 input_vmesh->get_center(p2,(VMesh::Node::index_type)(*two_new_tets)(0,1));
	
	if((*two_new_tets)(0,2)>=input_vmesh->num_nodes())
	{
	 p3=out;
	} else
	 input_vmesh->get_center(p3,(VMesh::Node::index_type)(*two_new_tets)(0,2));
	
	if((*two_new_tets)(0,3)>=input_vmesh->num_nodes())
	{
	  p4=out;
	} else
	 input_vmesh->get_center(p4,(VMesh::Node::index_type)(*two_new_tets)(0,3));
	 			
        edge_lengths=getEdgeLengths(p1, p2, p3, p4); /// find the longest edge
	std::vector<int> pos2 = maxi(edge_lengths);
	if (pos.size()==2)
	{
	  error("This message should not appear");
	}
	
	ComputeEdgeMidPoint(pos2[0], p1, p2, p3, p4, out);
        onodes[0]=(VMesh::Node::index_type)(*two_new_tets)(0,0);
	onodes[1]=(VMesh::Node::index_type)(*two_new_tets)(0,1);
	onodes[2]=(VMesh::Node::index_type)(*two_new_tets)(0,2);
	onodes[3]=(VMesh::Node::index_type)(*two_new_tets)(0,3);
	SplitTet(onodes, pos2[0], two_new_tets, node_count);
        (*new_nodes)(node_count  ,0)=out.x(); 
        (*new_nodes)(node_count  ,1)=out.y(); 
        (*new_nodes)(node_count++,2)=out.z(); 
	(*new_tets)(tet_count  ,0)=(*two_new_tets)(0,0); 
        (*new_tets)(tet_count  ,1)=(*two_new_tets)(0,1); 
        (*new_tets)(tet_count  ,2)=(*two_new_tets)(0,2); 
        (*new_tets)(tet_count++,3)=(*two_new_tets)(0,3);
	(*new_tets)(tet_count  ,0)=(*two_new_tets)(1,0); 
        (*new_tets)(tet_count  ,1)=(*two_new_tets)(1,1); 
        (*new_tets)(tet_count  ,2)=(*two_new_tets)(1,2); 
        (*new_tets)(tet_count++,3)=(*two_new_tets)(1,3);
	break;
      } else
      if (ind==1)
      {   
        
        (*new_tets)(tet_count  ,0)=(*two_new_tets)(0,0); /// save the tet that will not be splitted because it does not contain the second large edge that is equal
        (*new_tets)(tet_count  ,1)=(*two_new_tets)(0,1); /// to the first one (but that has been already been split)
        (*new_tets)(tet_count  ,2)=(*two_new_tets)(0,2); 
        (*new_tets)(tet_count++,3)=(*two_new_tets)(0,3);
	if((*two_new_tets)(1,0)>=input_vmesh->num_nodes())  /// get the node locations for the tet to be split
	{
	 p1=out;
	} else
   	 input_vmesh->get_center(p1,(VMesh::Node::index_type)(*two_new_tets)(1,0));

        if((*two_new_tets)(1,1)>=input_vmesh->num_nodes())  
	{
	 p2=out;
	} else
	 input_vmesh->get_center(p2,(VMesh::Node::index_type)(*two_new_tets)(1,1));
	
	if((*two_new_tets)(1,2)>=input_vmesh->num_nodes())
	{
	 p3=out;
	} else
	 input_vmesh->get_center(p3,(VMesh::Node::index_type)(*two_new_tets)(1,2));
	
	if((*two_new_tets)(1,3)>=input_vmesh->num_nodes())
	{
	  p4=out;
	} else
	 input_vmesh->get_center(p4,(VMesh::Node::index_type)(*two_new_tets)(1,3));
	 			
        edge_lengths=getEdgeLengths(p1, p2, p3, p4); /// find the longest edge
	std::vector<int> pos2 = maxi(edge_lengths);
	if (pos.size()==2)
	{
	  error("This message should not appear");
	}
	ComputeEdgeMidPoint(pos2[0], p1, p2, p3, p4, out);
        onodes[0]=(VMesh::Node::index_type)(*two_new_tets)(1,0);
	onodes[1]=(VMesh::Node::index_type)(*two_new_tets)(1,1);
	onodes[2]=(VMesh::Node::index_type)(*two_new_tets)(1,2);
	onodes[3]=(VMesh::Node::index_type)(*two_new_tets)(1,3);
	SplitTet(onodes, pos2[0], two_new_tets, node_count);
        (*new_nodes)(node_count  ,0)=out.x(); 
        (*new_nodes)(node_count  ,1)=out.y(); 
        (*new_nodes)(node_count++,2)=out.z(); 
	(*new_tets)(tet_count  ,0)=(*two_new_tets)(0,0); 
        (*new_tets)(tet_count  ,1)=(*two_new_tets)(0,1); 
        (*new_tets)(tet_count  ,2)=(*two_new_tets)(0,2); 
        (*new_tets)(tet_count++,3)=(*two_new_tets)(0,3);
	(*new_tets)(tet_count  ,0)=(*two_new_tets)(1,0); 
        (*new_tets)(tet_count  ,1)=(*two_new_tets)(1,1); 
        (*new_tets)(tet_count  ,2)=(*two_new_tets)(1,2); 
        (*new_tets)(tet_count++,3)=(*two_new_tets)(1,3);
       } else
      {
        error("Second long edge could not be found. This message should not appear.");
      }
      */
      break;
    }
   }
   
 }
 
 // create new field
 // add all nodes of old field to new field 
 // add ONLY tets from old field that were not selected - including the original field data value 
 // add all new tets and assign their field value (tet_field_values)
 
 return result;
}

bool RefineCleaverMeshAlgorithm::runImpl(FieldHandle input, DenseMatrixHandle matrix, FieldHandle& output) const
{
  // TODO: Check fields, matrix
  if (!input)
  {
   error("First module input (Field) is empty.");
   return false;
  }
  
  FieldInformation fi(input);
  
  if (!(fi.is_tetvolmesh()))
  {
   error("The input field data needs to be a tetvolmesh generated by the cleaver meshing package.");
  }

  if (!(fi.is_constantdata()))
  {
   error("The input field data (first module input) need to be located at the elements");
  }
  
  using namespace Parameters;
  int RadioButtonChoice = get(RefineCleaverMeshRadioButtons).toInt();
  int count=0;
  // start a loop here until selection_vector has no 1s - selection criteria is fulfilled?
  
  std::vector<bool> selection_vector = SelectMeshElements(input, matrix, RadioButtonChoice, count);
  
  if (selection_vector.size()==0 || count==0)
  {
   std::ostringstream ostr;
   ostr << " Based on the input data no tetrahedral element was found that could be split." << std::endl;
   remark(ostr.str());
   return false; 
  }
  
  std::vector<long> elems_to_split;
  elems_to_split.resize(count);

  count=0;
  for (long i=0;i<selection_vector.size();i++)
  {
   if(selection_vector[i])
   {
    elems_to_split[count++]=i;
   }
  }

  FieldHandle result=RefineMesh(input, elems_to_split);
  if(!result)
  {
   
  }
  
  //joinfields to get rid of duplicate nodes
  
  return (true);
}



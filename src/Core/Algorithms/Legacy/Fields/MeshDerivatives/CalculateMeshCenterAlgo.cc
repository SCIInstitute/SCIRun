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


#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/CalculateMeshCenterAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <Core/GeometryPrimitives/Vector.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, Method);

CalculateMeshCenterAlgo::CalculateMeshCenterAlgo()
{
  // set parameters defaults UI
  addOption(Parameters::Method,"weightedElemCenter","nodeCenter|elemCenter|weightedElemCenter|boundingBoxCenter|midNodeIndex|midElemIndex");
}


AlgorithmOutput CalculateMeshCenterAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  FieldHandle output;

  //pull parameter from UI
  std::string method=getOption(Parameters::Method);


  //Safety Check
  if(!inputField)
  {
    THROW_ALGORITHM_INPUT_ERROR("No input field");
  }

  // Get the information of the input field
  FieldInformation fo(inputField);
  fo.make_pointcloudmesh();
  fo.make_nodata();

  VMesh* imesh=inputField->vmesh();

  if(imesh->num_nodes()==0)
  {
    warning("Input field does contain any nodes, output will be an empty field");
    output=CreateField(fo);
    AlgorithmOutput result;
    result[Variables::OutputField] = output ;
    return result;
  }

  Point center(0.0,0.0,0.0);

  if(method=="nodeCenter")
  {
    Point c(0.0,0.0,0.0);
    VField::size_type numNodes=imesh->num_nodes();
    for(VMesh::Node::index_type idx=0;idx<numNodes;idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      c=Point(c+p);
    }
    if(numNodes)
    {
      center=(c*(1.0/static_cast<double>(numNodes)));
    }
  }
  else if(method=="elemCenter")
  {
    Point c(0.0,0.0,0.0);
    VField::size_type numElems=imesh->num_elems();
    for(VMesh::Elem::index_type idx=0;idx<numElems;idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      c=Point(c+p);
    }
    center=(c*(1.0/static_cast<double>(numElems)));
  }
  else if(method=="weightedElemCenter")
  {
    Point c(0.0,0.0,0.0);
    VField::size_type numElems=imesh->num_elems();
    double size=0.0;
    double weight;
    for(VMesh::Elem::index_type idx=0;idx<numElems;idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      weight=fabs(imesh->get_size(idx));
      size+=weight;
      c=Point(c+weight*p);
    }
    center=(c*(1.0/size));
  }
  else if(method=="boundingBoxCenter")
  {
    BBox b=imesh->get_bounding_box();
    center=(0.5*(b.get_max()+b.get_min())).point();
  }
  else if(method=="midNodeIndex")
  {
    VMesh::Node::index_type midIdx=(imesh->num_nodes()>>1);
    imesh->get_center(center,midIdx);
  }
  else if(method=="midElemIndex")
  {
    VMesh::Elem::index_type midIdx=(imesh->num_elems()>>1);
    imesh->get_center(center,midIdx);
  }

  output=CreateField(fo);
  output->vmesh()->add_point(center);
  output->vfield()->resize_values();

  AlgorithmOutput result;
  result[Variables::OutputField] = output ;
  return result;
}

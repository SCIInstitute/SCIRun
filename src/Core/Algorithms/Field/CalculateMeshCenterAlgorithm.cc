/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
#include <Core/Algorithms/Field/CalculateMeshCenterAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
//#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
//#include <boost/unordered_map.hpp>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
//using namespace SCIRun::Core::Geometry;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(Math, Method);

CalculateMeshCenterAlgorithm::CalculateMeshCenterAlgorithm()
{
  // set parameters defaults UI
  addOption(Paramters::Method,"weightedEleCenter","nodeCenter|eleCenter|weightedEleCenter|boundingBoxCenter|midNodeIndex|midEleIndex");
}


AlgorithmOutput CalculateMeshCenterAlgorithm::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  
  FieldHandle output;
  
  //pull parameter from UI
  std::string method=getOption(Paramters::Method);
  
  //Safety Check
  if(inputField.getRep()==0)
  {
    error("No input field");
    return nullptr;
  }
  
  FieldInformation fo(inputField);
  fo.makePointCloudMesh();
  fo.make_nodata();
  
  if(inputField->numNodes()==0)
  {
    warning("Input field does contain any nodes, output will be an empty field");
    output=CreateField(fo);
    return output;
  }
  
  VMesh* imesh=inputField->vmesh();
  
  MeshHandle mesh=CreateMesh(fo);
  
  if(imesh->numNodes()=0)
  {
    warning("Input field does contain any nodes, output will be an empty field");
    output=CreateField(fo);
    return output;
  }
  
  Point center(0.0,0.0,0.0);
  
  if(method_=="nodeCenter")
  {
    Point c(0.0,0.0,0.0);
    Vfield::sizeType numNodes=imesh->NumNodes();
    for(Vfield::Node::indexType idx=0;idx<numNodes;idx++)
    {
      Point p;
      imesh->getCenter(p,idx);
      c=Point(c+p);
    }
    if(numNodes)
    {
      center=(c*(1.0/static_cast<double>(numNodes)));
    }
  }
  else if(method=="eleCenter")
  {
    Point c(0.0,0.0,0.0);
    VField::sizeType numElems=imesh->numElems();
    for(VMesh::Elem::indexType idx=0;idx<numElems;idx++)
    {
      Point p;
      imesh->getCenter(p,idx);
      c=Point(c+p);
    }
    center=(c*(1.0/static_cast<double>(numElems)));
  }
  else if(method=="weightedEleCenter")
  {
    Point c(0.0,0.0,0.0);
    VField::sizeType numElems=imesh->numElems();
    double size=0.0;
    double weight;
    for(VMesh::Elem::indexType idx=0;idx<numElems;idx++)
    {
      Point p;
      imesh->getCenter(p,idx);
      weight=Abs(imesh->getSize(idx));
      size+=weight;
      c=Point(c+weight*p);
    }
    center=(c*(1.0/size));
  }
  else if(method=="boundingBoxCenter")
  {
    BBox b=imesh->getBoundingBox();
    center=(0.5*(b.max()+b.min())).point();
  }
  else if(method=="midNodeIndex")
  {
    VMesh::Node::indexType midIdx=(imesh->numNodes()>>1);
    imesh->getCenter(center,midIdx);
  }
  else if(method=="midELeIndex")
  {
    VMesh::Ele::indexType midIdx=(imesh->numElems()>>1);
    imesh->getCenter(center,midIdx);
  }
  
  output=CreateField(fo);
  output.vmesh.addPoint(center);
  output.Vfield().resizeValues();
  
  AlgorithmOutput result;
  result[Variables::OutputField] = output ;
  return result;
}

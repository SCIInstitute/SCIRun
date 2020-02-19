/*/*
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


#include <Core/Algorithms/Legacy/Fields/Cleanup/ReorderNormalCoherentlyAlgo.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include<Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include<queue>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::Fields::Parameters;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(Fields, invertedElementsCheckBox);

ReorderNormalCoherentlyAlgo::ReorderNormalCoherentlyAlgo()
{
  addParameter(Parameters::invertedElementsCheckBox, false);
}

void ReorderNormalCoherentlyAlgo::runImpl(FieldHandle inputField, FieldHandle& outputField, DenseColumnMatrixHandle& invertedElementsListMatrix) const
{
  if(!inputField)
  {
    THROW_ALGORITHM_INPUT_ERROR("No input field");
  }

  FieldInformation fi(inputField);

  if(fi.is_nonlinear())
    error("The algorithm has not yet been defined for non-linear elements yet");

  if(!fi.is_trisurfmesh())
    error("This algorithm only works on a TriSurfMesh");

  VField* inputVField=inputField->vfield();
  VMesh* inputVMesh=inputField->vmesh();

  fi.make_trisurfmesh();
  outputField=CreateField(fi);

  if(!outputField)
    error("Could not allocate output field");

  VMesh *outputVMesh=outputField->vmesh();
  const VMesh::size_type n=inputVMesh->num_nodes();
  const VMesh::size_type m=inputVMesh->num_elems();
  outputVMesh->node_reserve(n);
  outputVMesh->elem_reserve(m);

  outputVMesh->copy_nodes(inputVMesh);

  size_type i,j,k;
  std::vector<int> invertedElements;

  std::vector<std::set<int>> elemsOfVert;
  std::vector<std::set<int>> elemNeighbors;
  std::set<int> dummy;
  std::vector<int> tempVec={0,0,0};
  std::vector<std::vector<int>> elem(m, tempVec);
  const int noOfV=3;

  //intialize vector of vectors

  VMesh::Face::iterator meshFaceIter;
  VMesh::Face::iterator meshFaceEnd;

  VMesh::Node::array_type nodesFromFace(noOfV);

  inputVMesh->synchronize(Mesh::ALL_ELEMENTS_E);

  inputVMesh->end(meshFaceEnd);

  for(inputVMesh->begin(meshFaceIter); meshFaceIter!=meshFaceEnd; ++meshFaceIter)
  {
    // get nodes from mesh element
    VMesh::Face::index_type elemID=*meshFaceIter;
    inputVMesh->get_nodes(nodesFromFace, elemID);

    for(i=0;i<noOfV;i++)
      elem[elemID][i]=nodesFromFace[i];
  }

  for(i=0;i<m;i++)
    elemNeighbors.push_back(dummy);

  for(i=0;i<n;i++)
    elemsOfVert.push_back(dummy);

  //get elements
  for(i=0;i<m;i++)
  {
    for(j=0;j<noOfV;j++)
      elemsOfVert[elem[i][j]].insert(i);
  }

  //get edges
  //create graph
  for(i=0;i<m;i++)
  {
    for(j=0;j<noOfV;j++)
    {
      k = (j+1)%noOfV;
      std::set<int>::iterator it;
      for (it = elemsOfVert[elem[i][j]].begin(); it != elemsOfVert[elem[i][j]].end(); it++)
      {
        if (elemsOfVert[elem[i][k]].find(*it) != elemsOfVert[elem[i][k]].end())
        {
          if (*it != i)
          {
            elemNeighbors[i].insert(*it);
            elemNeighbors[i].insert(*it);
          }
        }
      }
    }
  }

    //traverse graph
    std::vector<bool> elemTraversed(m, false);

    std::queue<int> bfs;
    bfs.push(0);
    elemTraversed[0]=true;
    std::set<std::pair<int,int>> edges;

  while(!bfs.empty())
  {
    //insert elems not traversed
    std::set<int>::iterator it;
    i=bfs.front();

    //invert in the output field
    VMesh::Face::index_type elemID=i;
    inputVMesh->get_nodes(nodesFromFace, elemID);

    for(it=elemNeighbors[i].begin();it!=elemNeighbors[i].end();it++)
    {
      if(!elemTraversed[*it])
      {
        bfs.push(*it);
        elemTraversed[*it]=true;
      }
    }
    //reordered elem i if desired
    bool flag=false;
    for (j = 0; j < noOfV && flag == false; j++)
    {
      k = (j+1)%noOfV;
      if (edges.find(std::make_pair(elem[i][j], elem[i][k])) != edges.end())
      {
        invertedElements.push_back(i);
        // invert ordering
        flag = true;
        int temp = elem[i][j];
        elem[i][j]=elem[i][k];
        elem[i][k]=temp;
        nodesFromFace[j] = elem[i][j];
        nodesFromFace[k] = elem[i][k];
      }
    }
    for (j = 0; j < noOfV; j++)
    {
      k=(j+1)%noOfV;
      edges.insert(std::make_pair(elem[i][j], elem[i][k]));
    }
    outputVMesh->add_elem(nodesFromFace);

    bfs.pop();
  }

  VField* outputVField=outputField->vfield();
  outputVField->resize_values();
  std::vector<VMesh::index_type> order;

    // Copy field data (non-linear not supoorted, check made upstream)
  int basis_order=inputVField->basis_order();
  if (basis_order == 0)
  {
    VField::size_type size = order.size();
    for(VField::index_type idx = 0; idx < size; idx++)
      outputVField->copy_value(inputVField, order[idx], idx);
  }
  else if (basis_order == 1)
  {
    outputVField->copy_values(inputVField);
  }

  CopyProperties(*inputField,*outputField);


  if (invertedElements.size() > 0)
  {
    std::ostringstream oss;
    oss << invertedElements.size() << "elements were found to be incorrectly oriented in input TriSurf mesh.";
    remark(oss.str());

    if (get(Parameters::invertedElementsCheckBox).toBool())
    {
      invertedElementsListMatrix = boost::make_shared<DenseColumnMatrix>(invertedElements.size());

      for(int i=0;i<invertedElements.size();i++)
      {

        (*invertedElementsListMatrix)(i)=invertedElements[i];
      }

    }
  }
  else
  {
    invertedElementsListMatrix=nullptr;
  }
}

AlgorithmOutput ReorderNormalCoherentlyAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);

  FieldHandle outputField;
  DenseColumnMatrixHandle outputMatrix;

  runImpl(inputField, outputField, outputMatrix);

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  output[Variables::OutputMatrix] = outputMatrix;

  return output;
}

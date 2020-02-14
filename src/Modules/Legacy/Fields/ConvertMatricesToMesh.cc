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


///
///@author
///   Michael Callahan,
///   Department of Computer Science,
///   University of Utah
///@date  February 2001
///

#include <Modules/Legacy/Fields/ConvertMatricesToMesh.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(ConvertMatricesToMesh, NewField, SCIRun)

ConvertMatricesToMesh::ConvertMatricesToMesh() : Module(staticInfo_)
{
  INITIALIZE_PORT(MeshElements);
  INITIALIZE_PORT(MeshPositions);
  INITIALIZE_PORT(MeshNormals);
  INITIALIZE_PORT(OutputField);
}

void ConvertMatricesToMesh::setStateDefaults()
{
  auto state = get_state();
  state->setValue(FieldBaseType, std::string("TetVol"));
  state->setValue(DataType, std::string("double"));

  state->setValue(InputFieldName, std::string("[Field Name]"));
  state->setValue(InputFieldTypeName, std::string("[Field Type]"));
}

void ConvertMatricesToMesh::execute()
{
  auto positionshandle = getRequiredInput(MeshPositions);
  auto normalshandle = getOptionalInput(MeshNormals);

  if (!normalshandle)
  {
    remark("No input normals connected, not used.");
  }

  if (needToExecute())
  {
    if (positionshandle->ncols() < 3)
    {
      error("Mesh Positions must contain at least 3 columns for position data.");
      return;
    }
    if (positionshandle->ncols() > 3)
    {
      remark("Mesh Positions contains unused columns, only first three are used.");
    }

    auto state = get_state();
    std::string basename = state->getValue(FieldBaseType).toString();
    std::string datatype = state->getValue(DataType).toString();

    FieldInformation fi("CurveMesh",1,datatype);
    if (basename == "Curve") fi.make_curvemesh();
    else if (basename == "HexVol") fi.make_hexvolmesh();
    else if (basename == "PointCloud") fi.make_pointcloudmesh();
    else if (basename == "PrismVol") fi.make_prismvolmesh();
    else if (basename == "QuadSurf") fi.make_quadsurfmesh();
    else if (basename == "TetVol") fi.make_tetvolmesh();
    else if (basename == "TriSurf") fi.make_trisurfmesh();

    FieldHandle result_field = CreateField(fi);
    VMesh* mesh = result_field->vmesh();

    const size_type pnrows = positionshandle->nrows();
    for (auto i = 0; i < pnrows; i++)
    {
      const Point p(positionshandle->get(i, 0),
        positionshandle->get(i, 1),
        positionshandle->get(i, 2));
      mesh->add_point(p);
    }

    bool meshElementsRequired = basename != "PointCloud";
    if (meshElementsRequired)
    {
      process_elements(mesh, pnrows);
    }

    result_field->vfield()->resize_values();
    sendOutput(OutputField, result_field);
  }
}

void ConvertMatricesToMesh::process_elements(VMesh* mesh, size_type positionRows)
{
  auto elementshandle = getRequiredInput(MeshElements);

  index_type ecount = 0;
  const size_type enrows = elementshandle->nrows();
  const size_type encols = elementshandle->ncols();
  VMesh::Node::array_type nodes;

  for (index_type i = 0; i < enrows; i++)
  {
    nodes.clear();
    for (index_type j = 0; j < encols; j++)
    {
      VMesh::Node::index_type index = static_cast<index_type>(elementshandle->get(i, j));
      if (index < 0 || index >= positionRows)
      {
        if (ecount < 10)
        {
          error("Bad index found at " + std::to_string(i) + ", " + std::to_string(j));
        }
        index = 0;
        ecount++;
      }
      nodes.push_back(index);
    }
    mesh->add_elem(nodes);
  }
  if (ecount >= 10)
  {
    error("..." + std::to_string(ecount - 9) + " additional bad indices found.");
  }
}

const AlgorithmParameterName ConvertMatricesToMesh::InputFieldName("InputFieldName");
const AlgorithmParameterName ConvertMatricesToMesh::InputFieldTypeName("InputFieldTypeName");
const AlgorithmParameterName ConvertMatricesToMesh::FieldBaseType("FieldBaseType");
const AlgorithmParameterName ConvertMatricesToMesh::DataType("DataType");

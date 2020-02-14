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


///@file  ReportFieldGeometryMeasures.cc
///
///@author
///   David Weinstein
///   Department of Computer Science
///   University of Utah
///@date  March 2001

#include <Modules/Legacy/Fields/ReportFieldGeometryMeasures.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Legacy/Fields/RegisterWithCorrespondences.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>


using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Fields;

/// @class ReportFieldGeometryMeasures
/// @brief Build a densematrix, where each row is a particular measure of the
/// input Field (e.g. the x-values, or the element size).

MODULE_INFO_DEF(ReportFieldGeometryMeasures, MiscField, SCIRun)

ALGORITHM_PARAMETER_DEF(Fields, MeasureLocation);
ALGORITHM_PARAMETER_DEF(Fields, XPositionFlag);
ALGORITHM_PARAMETER_DEF(Fields, YPositionFlag);
ALGORITHM_PARAMETER_DEF(Fields, ZPositionFlag);
ALGORITHM_PARAMETER_DEF(Fields, IndexFlag);
ALGORITHM_PARAMETER_DEF(Fields, SizeFlag);
ALGORITHM_PARAMETER_DEF(Fields, NormalsFlag);

ReportFieldGeometryMeasures::ReportFieldGeometryMeasures() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(Output_Measures);
}

void ReportFieldGeometryMeasures::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::MeasureLocation, std::string("Nodes"));
  state->setValue(Parameters::XPositionFlag, true);
  state->setValue(Parameters::YPositionFlag, true);
  state->setValue(Parameters::ZPositionFlag, true);
  state->setValue(Parameters::IndexFlag, false);
  state->setValue(Parameters::SizeFlag, false);
  state->setValue(Parameters::NormalsFlag, false);
}

void ReportFieldGeometryMeasures::execute()
{
  auto fieldhandle = getRequiredInput(InputField);

  auto state = get_state();
  VMesh* mesh = fieldhandle->vmesh();

  /// This is a hack for now, it is definitely not an optimal way
  int syncflag = 0;
  std::string simplex = state->getValue(Parameters::MeasureLocation).toString();

  if (simplex == "Elements")
  {
    if (mesh->dimensionality() == 0) simplex = "Node";
    else if (mesh->dimensionality() == 1) simplex = "Edge";
    else if (mesh->dimensionality() == 2) simplex = "Face";
    else if (mesh->dimensionality() == 3) simplex = "Cell";
  }

  if (simplex == "Nodes")
    syncflag = Mesh::NODES_E | Mesh::NODE_NEIGHBORS_E;
  else if (simplex == "Edges")
    syncflag = Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E;
  else if (simplex == "Faces")
    syncflag = Mesh::FACES_E | Mesh::ELEM_NEIGHBORS_E;
  else if (simplex == "Cells")
    syncflag = Mesh::CELLS_E;

  mesh->synchronize(syncflag);

  const bool normalsFlag = state->getValue(Parameters::NormalsFlag).toBool();
  bool nnormals = normalsFlag && (state->getValue(Parameters::MeasureLocation).toString() == "Nodes");
  bool fnormals = normalsFlag && (state->getValue(Parameters::MeasureLocation).toString() == "Faces");

  if (nnormals && !mesh->has_normals())
  {
    warning("This mesh type does not contain normals, skipping.");
    nnormals = false;
  }
  else if (fnormals && !mesh->has_normals())
  {
    warning("This mesh type does not contain normals, skipping.");
    fnormals = false;
  }
  else if (normalsFlag && !(nnormals || fnormals))
  {
    warning("Cannot compute normals at that simplex location, skipping.");
  }

  if (nnormals || fnormals)
  {
    mesh->synchronize(Mesh::NORMALS_E);
  }

  const bool x = state->getValue(Parameters::XPositionFlag).toBool();
  const bool y = state->getValue(Parameters::YPositionFlag).toBool();
  const bool z = state->getValue(Parameters::ZPositionFlag).toBool();
  const bool eidx = state->getValue(Parameters::IndexFlag).toBool();
  const bool size = state->getValue(Parameters::SizeFlag).toBool();

  size_type ncols=0;
  if (x)     ncols++;
  if (y)     ncols++;
  if (z)     ncols++;
  if (eidx)   ncols++;
  if (size)  ncols++;
  if (nnormals) ncols+=3;
  if (fnormals) ncols+=3;

  if (ncols==0)
  {
    error("No measures selected.");
    return;
  }

  DenseMatrixHandle output;

  if (state->getValue(Parameters::MeasureLocation).toString() == "Nodes")
  {
    VMesh::Node::size_type nrows;
    mesh->size(nrows);
    output.reset(new DenseMatrix(nrows,ncols));
    double* dataptr = output->data();

    Point p; double vol;
    for(VMesh::Node::index_type idx=0; idx<nrows; idx++)
    {
      mesh->get_center(p,idx);
      vol = mesh->get_size(idx);
      if (x) { *dataptr = p.x(); dataptr++; }
      if (y) { *dataptr = p.y(); dataptr++; }
      if (z) { *dataptr = p.z(); dataptr++; }
      if (eidx) { *dataptr = static_cast<double>(idx); dataptr++; }
      if (size) { *dataptr = vol; dataptr++; }
      if (nnormals)
      {
        Vector v; mesh->get_normal(v,idx);
        dataptr[0] = v.x(); dataptr[1] = v.y();
        dataptr[2] = v.z(); dataptr += 3;
      }
    }
  }
  else if (state->getValue(Parameters::MeasureLocation).toString() == "Edges")
  {
    VMesh::Edge::size_type nrows;
    mesh->size(nrows);
    output.reset(new DenseMatrix(nrows, ncols));
    double* dataptr = output->data();

    Point p; double vol;
    for(VMesh::Edge::index_type idx=0; idx<nrows; idx++)
    {
      mesh->get_center(p,idx);
      vol = mesh->get_size(idx);
      if (x) { *dataptr = p.x(); dataptr++; }
      if (y) { *dataptr = p.y(); dataptr++; }
      if (z) { *dataptr = p.z(); dataptr++; }
      if (eidx) { *dataptr = static_cast<double>(idx); dataptr++; }
      if (size) { *dataptr = vol; dataptr++; }
    }
  }
  else if (state->getValue(Parameters::MeasureLocation).toString() == "Faces")
  {
    VMesh::Face::size_type nrows;
    mesh->size(nrows);
    remark("Entered");
    output.reset(new DenseMatrix(nrows, ncols));
    double* dataptr = output->data();

    Point p; double vol;
    VMesh::coords_type center;
    mesh->get_element_center(center);

    for(VMesh::Face::index_type idx=0; idx<nrows; idx++)
    {
      mesh->get_center(p,idx);
      vol = mesh->get_size(idx);
      if (x) { *dataptr = p.x(); dataptr++; }
      if (y) { *dataptr = p.y(); dataptr++; }
      if (z) { *dataptr = p.z(); dataptr++; }
      if (eidx) { *dataptr = static_cast<double>(idx); dataptr++; }
      if (size) { *dataptr = vol; dataptr++; }
      if (fnormals)
      {
        Vector v; mesh->get_normal(v,center,VMesh::Elem::index_type(idx));
        dataptr[0] = v.x(); dataptr[1] = v.y();
        dataptr[2] =v.z(); dataptr += 3;
      }
    }
  }
  else if (state->getValue(Parameters::MeasureLocation).toString() == "Cells")
  {
    VMesh::Cell::size_type nrows;
    mesh->size(nrows);
    output.reset(new DenseMatrix(nrows, ncols));
    double* dataptr = output->data();

    Point p; double vol;
    for(VMesh::Cell::index_type idx=0; idx<nrows; idx++)
    {
      mesh->get_center(p,idx);
      vol = mesh->get_size(idx);
      if (x) { *dataptr = p.x(); dataptr++; }
      if (y) { *dataptr = p.y(); dataptr++; }
      if (z) { *dataptr = p.z(); dataptr++; }
      if (eidx) { *dataptr = static_cast<double>(idx); dataptr++; }
      if (size) { *dataptr = vol; dataptr++; }
    }
  }

  if (output && output->nrows() == 0)
  {
    warning("No measures added to matrix--output will be an empty matrix with zero rows.");
  }

  sendOutput(Output_Measures, output);
}

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
//#include <Core/Datatypes/MatrixTypeConverter.h>

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

const ModuleLookupInfo ReportFieldGeometryMeasures::staticInfo_("ReportFieldGeometryMeasures", "MiscField", "SCIRun");

ALGORITHM_PARAMETER_DEF(Fields, MeasureLocation);
ALGORITHM_PARAMETER_DEF(Fields, XPositionFlag);
ALGORITHM_PARAMETER_DEF(Fields, YPositionFlag);
ALGORITHM_PARAMETER_DEF(Fields, ZPositionFlag);
ALGORITHM_PARAMETER_DEF(Fields, IndexFlag);
ALGORITHM_PARAMETER_DEF(Fields, SizeFlag);
ALGORITHM_PARAMETER_DEF(Fields, NormalsFlag);

#if 0
    GuiString simplexString_;
    GuiInt xFlag_;
    GuiInt yFlag_;
    GuiInt zFlag_;
    GuiInt idxFlag_;
    GuiInt sizeFlag_;
    GuiInt normalsFlag_;
#endif

ReportFieldGeometryMeasures::ReportFieldGeometryMeasures() : Module(staticInfo_)
  /*
    simplexString_(get_ctx()->subVar("simplexString"), "Node"),
    xFlag_(get_ctx()->subVar("xFlag"), 1), 
    yFlag_(get_ctx()->subVar("yFlag"), 1),
    zFlag_(get_ctx()->subVar("zFlag"), 1), 
    idxFlag_(get_ctx()->subVar("idxFlag"), 0),
    sizeFlag_(get_ctx()->subVar("sizeFlag"), 0),
    normalsFlag_(get_ctx()->subVar("normalsFlag"), 0)*/
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

  VMesh* mesh = fieldhandle->vmesh();
#if 0
  /// This is a hack for now, it is definitely not an optimal way
  int syncflag = 0;
  std::string simplex =simplexString_.get();
  
  if (simplex == "Elem")
  {
    if (mesh->dimensionality() == 0) simplex = "Node";
    else if (mesh->dimensionality() == 1) simplex = "Edge";
    else if (mesh->dimensionality() == 2) simplex = "Face";
    else if (mesh->dimensionality() == 3) simplex = "Cell";
  }

  if (simplex == "Node")
    syncflag = Mesh::NODES_E | Mesh::NODE_NEIGHBORS_E;
  else if (simplex == "Edge")
    syncflag = Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E;
  else if (simplex == "Face")
    syncflag = Mesh::FACES_E | Mesh::ELEM_NEIGHBORS_E;
  else if (simplex == "Cell")
    syncflag = Mesh::CELLS_E;

  mesh->synchronize(syncflag);

  bool nnormals = normalsFlag_.get() && (simplexString_.get() == "Node");
  bool fnormals = normalsFlag_.get() && (simplexString_.get() == "Face");

  if (nnormals && !mesh->has_normals())
  {
    warning("This mesh type does not contain normals, skipping.");
    nnormals = false;
  }
  else if (normalsFlag_.get() && !(nnormals || fnormals))
  {
    warning("Cannot compute normals at that simplex location, skipping.");
  }
  
  if (nnormals)
  {
    mesh->synchronize(Mesh::NORMALS_E);
  }

  bool x = xFlag_.get();
  bool y = yFlag_.get();
  bool z = zFlag_.get();
  bool eidx = idxFlag_.get();
  bool size = sizeFlag_.get();

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

  MatrixHandle output;

  update_state(Executing);

  if (simplexString_.get() == "Node")
  {
    VMesh::Node::size_type nrows;
    mesh->size(nrows);
    output = new DenseMatrix(nrows,ncols);
    double* dataptr = output->get_data_pointer();
  
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
  else if (simplexString_.get() == "Edge")
  {
    VMesh::Edge::size_type nrows;
    mesh->size(nrows);
    output = new DenseMatrix(nrows,ncols);
    double* dataptr = output->get_data_pointer();
  
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
  else if (simplexString_.get() == "Face")
  {
    VMesh::Face::size_type nrows;
    mesh->size(nrows);
    output = new DenseMatrix(nrows,ncols);
    double* dataptr = output->get_data_pointer();
  
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
  else if (simplexString_.get() == "Cell")
  {
    VMesh::Cell::size_type nrows;
    mesh->size(nrows);
    output = new DenseMatrix(nrows,ncols);
    double* dataptr = output->get_data_pointer();
  
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

  send_output_handle("Output Measures Matrix", output, true);
#endif
}

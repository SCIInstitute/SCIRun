/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2011 Scientific Computing and Imaging Institute,
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
 *  BuildBEMatrix.cc:
 *
 *  Written by:
 *   Saeed Babaeizadeh - Northeastern University
 *   Michael Callahan - Department of Computer Science - University of Utah
 *   May, 2003
 *
 *  Updated by:
 *   Burak Erem - Northeastern University
 *   January, 2012
 *
 */

//#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystem.h>

#include <Modules/Legacy/Forward/BuildBEMatrix.h>
#include <Core/Algorithms/Legacy/Forward/BuildBEMatrixAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>

#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Math/MiscMath.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>

using namespace SCIRun;
using namespace SCIRun::Modules::Forward;

namespace balgo=boost::algorithm;

namespace detail {

enum BEM_ALGO_TYPE {
  UNSUPPORTED = -1,
  SURFACE_AND_POINTS = 1,
  SURFACES_TO_SURFACES
};

// Unfortunately, there is no clean way to map this to TCL.
// See method ui for code that sets up radiobuttons
// in BuildBEMatrix.tcl.
enum GUI_SURFACE_TYPE {
  SOURCE = 0,
  MEASUREMENT
};

// TODO: lets use intercap for class name when it's moved to an algorithm
//
// initial conditions: first should be measurement, rest source



class BuildBEMatrix : public Module, public BuildBEMatrixBase
{
public:

  virtual void execute();

private:
  // BURAK EDITS:
  void algoSurfaceToNodes();
  void algoSurfacesToSurfaces();
  int detectBEMalgo();

  DenseMatrixHandle TransferMatrix;

  MatrixHandle hZoi_;
  int old_nodes_generation_;
  int old_surface_generation_;
  bool process_gui_vars_;

  GuiString guiselected_field_;
  GuiString guiselected_type_;

  GuiString guifields_;
  GuiString guifield_type_property_;
  GuiString guifield_inside_cond_property_;
  GuiString guifield_outside_cond_property_;
  GuiString guifield_surface_type_property_;

  bemfield_vector fields_;
};

// TODO: move into class?
std::vector<int> field_generation_no_old, old_nesting;
std::vector<double> old_conductivities;

BuildBEMatrix::BuildBEMatrix(GuiContext *context):
  Module("BuildBEMatrix", context, Source, "Forward", "BioPSE"),
  old_nodes_generation_( -1 ),
  old_surface_generation_( -1 ),
  process_gui_vars_(false),
  guiselected_field_(get_ctx()->subVar("selected_field", false), ""),
  guiselected_type_(get_ctx()->subVar("selected_field_type", false), ""),
  guifields_(get_ctx()->subVar("input_field_list"), ""),
  guifield_type_property_(get_ctx()->subVar("field_type_list"), ""),
  guifield_inside_cond_property_(get_ctx()->subVar("inside_cond_list"), ""),
  guifield_outside_cond_property_(get_ctx()->subVar("outside_cond_list"), ""),
  guifield_surface_type_property_(get_ctx()->subVar("surface_type_list"), "")
{}

// BURAK EDITS:

void
BuildBEMatrix::execute()
{
  std::vector<FieldHandle> inputs;
  get_dynamic_input_handles("Surface", inputs, true);

  const std::size_t INPUTS_LEN = inputs.size(), INPUT_LEN_LAST_INDEX = INPUTS_LEN - 1;

  if (this->inputs_changed_)
  {
    this->fields_.clear();
    this->fields_.reserve(INPUTS_LEN);

    for (std::vector<FieldHandle>::size_type i = 0; i < INPUTS_LEN; ++i)
    {
      FieldHandle field_handle = inputs[i];
      // TODO: from old code - needed?
      field_handle.detach();
      field_handle->mesh_detach();

      bemfield field(field_handle);

      // don't think the strings need to be quoted, but if they do,
      // quote using {}
      fieldlist << "field" << i;

      // setting field type
      VMesh* vmesh = inputs[i]->vmesh();
      if (vmesh->is_trisurfmesh())
      {
        field.surface = true;
        fieldtype << "surface";
      }
      else if (vmesh->is_pointcloudmesh())
      {
        // probably redundant...
        field.surface = false;
        fieldtype << "points";
      }
      else
      {
        // unsupported field types
        warning("Input field in not either a TriSurf mesh or a PointCloud.");
        fieldtype << "unknown";
      }

      this->fields_.push_back(field);
    }
  }

  if (true)
  {


          this->fields_[i].insideconductivity = boost::lexical_cast<double>(split_vector[i]);

          this->fields_[i].outsideconductivity = boost::lexical_cast<double>(split_vector[i]);



          int surface_type = boost::lexical_cast<int>(split_vector[i]);
          if (surface_type == SOURCE)
          {
            this->fields_[i].set_source_dirichlet();
          }
          else // measurement
          {
            this->fields_[i].set_measuremen_neumann();
          }

  }

  // The specific BEM routine (2 so far) to be called is dependent on the inputs in the fields vector,
  // so we check for the conditions and call the appropriate routine:
  int BEMalgo = detectBEMalgo();

  switch(BEMalgo)
  {
  case UNSUPPORTED:
    // We don't support the inputs and detectBEMalgo() should have reported the appropriate error, so just return
    //
    // TODO: error message needs improvement
    error("The combinations of input properties is not supported. Please see documentation for supported input field options.");
    return;

  case SURFACE_AND_POINTS:
    // BEMalgo == 1 means we've detected the special case of two fields with a surface outside and points inside
    algoSurfaceToNodes();
	break;

  case SURFACES_TO_SURFACES:
    // BEMalgo == 2 means we've detected the surfaces to surfaces case
    algoSurfacesToSurfaces();
	break;

  default:
    // Should never happen, but return with an error
    error("Unable to detect the appropriate algorithm and the reason is unknown. Please report this error to the developers.");
    return;
  }

  // The BEM algorithms populate a matrix with handle "TransferMatrix". At this point all we have to do is send it as output.
  send_output_handle("BEM Forward Matrix", TransferMatrix);

}

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

#include <Modules/Legacy/Forward/BuildBEMatrixImpl.h>
#include <Core/Algorithms/Legacy/Forward/BuildBEMatrixAlgo.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Logging/LoggerInterface.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Forward;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Forward;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;

BuildBEMatrixImpl::BuildBEMatrixImpl(const VariableList& names,
  const VariableList& bdyConds,
  const VariableList& outside,
  const VariableList& inside,
  LegacyLoggerInterface* log) :
  names_(names),
  bdyConds_(bdyConds),
  outside_(outside),
  inside_(inside),
  log_(log)
{

}

MatrixHandle BuildBEMatrixImpl::executeImpl(const FieldList& inputs)
{
  bemfield_vector fields;

  for (FieldHandle input : inputs)
  {
    bemfield field(input);

    // setting field type
    VMesh* vmesh = input->vmesh();
    if (vmesh->is_trisurfmesh())
    {
      field.surface = true;
      inputTypes_.push_back("surface");
    }
    else if (vmesh->is_pointcloudmesh())
    {
      // probably redundant...
      field.surface = false;
      inputTypes_.push_back("points");
    }
    else
    {
      // unsupported field types
      log_->warning("Input field in not either a TriSurf mesh or a PointCloud.");
      inputTypes_.push_back("unknown");
    }

    fields.push_back(field);
  }

  if (names_.size() != fields.size())
    THROW_INVALID_ARGUMENT("# of Input fields does match # of names from table.");
  if (bdyConds_.size() != fields.size())
    THROW_INVALID_ARGUMENT("# of Input fields does match # of boundary conditions from table.");
  if (outside_.size() != fields.size())
    THROW_INVALID_ARGUMENT("# of Input fields does match # of outside conductivities from table.");
  if (inside_.size() != fields.size())
    THROW_INVALID_ARGUMENT("# of Input fields does match # of inside conductivities from table.");

  {
    for (size_t i = 0; i < fields.size(); ++i)
    {
      bemfield& field = fields[i];
      field.insideconductivity = inside_[i].toDouble();
      field.outsideconductivity = outside_[i].toDouble();

      std::string surface_type = bdyConds_[i].toString();
      if (surface_type.find("Dirichlet") != std::string::npos)
      {
        field.set_source_dirichlet();
      }
      else // measurement
      {
        field.set_measurement_neumann();
      }
    }
  }

  // The specific BEM routine (2 so far) to be called is dependent on the inputs in the fields vector,
  // so we check for the conditions and call the appropriate routine:
  auto BEMalgo = BEMAlgoImplFactory::create(fields);

  if (!BEMalgo)
  {
    // TODO: error message needs improvement
    log_->error("The combinations of input properties is not supported. Please see documentation for supported input field options.");
    return nullptr;
  }
  return BEMalgo->compute(fields);
}

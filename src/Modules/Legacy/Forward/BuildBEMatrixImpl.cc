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

#include <Modules/Legacy/Forward/BuildBEMatrixImpl.h>
#include <Core/Algorithms/Legacy/Forward/BuildBEMatrixAlgo.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Forward;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Forward;

MatrixHandle BuildBEMatrixImpl::executeImpl(const FieldList& inputs) const
{
    bemfield_vector fields;

    BOOST_FOREACH(FieldHandle input, inputs)
    {
      bemfield field(field_handle);

      // setting field type
      VMesh* vmesh = input->vmesh();
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

      fields.push_back(field);
    }


//TODO
#if 0
  // set flags based on gui table values
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
  #endif

  // The specific BEM routine (2 so far) to be called is dependent on the inputs in the fields vector,
  // so we check for the conditions and call the appropriate routine:
  auto BEMalgo = BEMAlgoImplFactory::create(fields);

  if (!BEMalgo)
  {
//  case UNSUPPORTED:
    // We don't support the inputs and detectBEMalgo() should have reported the appropriate error, so just return
    //
    // TODO: error message needs improvement
    error("The combinations of input properties is not supported. Please see documentation for supported input field options.");
    return;
  }
  return BEMalgo->compute(fields);
}

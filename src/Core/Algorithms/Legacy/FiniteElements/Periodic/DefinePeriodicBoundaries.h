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


#ifndef CORE_ALGORITHMS_FINITEELEMENTS_DEFINEPERIODICBOUNDARIES_H
#define CORE_ALGORITHMS_FINITEELEMENTS_DEFINEPERIODICBOUNDARIES_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>

// Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

// for Windows support
#include <Core/Algorithms/FiniteElements/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

// Two algorithms:
// The first one generates how nodes of two opposing boundaries could link together
// and how elements fit together.
// The second second does the same but uses the information on the field to see
// which element should connect to which.

class SCISHARE DefinePeriodicBoundariesAlgo : public AlgoBase
{
  public:
    DefinePeriodicBoundariesAlgo()
    {
      /// Link boundaries across x coordinate
      add_bool("link_x_boundary",true);
      /// Link boundaries across y coordinate
      add_bool("link_y_boundary",true);
      /// Link boundaries across z coordinate
      add_bool("link_z_boundary",true);
      /// Which tolerance to use for matching nodes (-1.0 will use epsilon of mesh)
      add_scalar("tolerance",-1.0);
      /// Whether to build the PeriodicNodeLink Mapping matrix
      add_bool("build_periodic_nodelink",true);
      add_bool("build_periodic_delemlink",true);
    }

    bool run(FieldHandle& input,
             MatrixHandle& periodic_nodelink,
             MatrixHandle& periodic_delemlink);
};


} // end namespace SCIRunAlgo

#endif

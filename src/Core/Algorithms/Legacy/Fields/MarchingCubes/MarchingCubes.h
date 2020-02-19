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


#ifndef CORE_ALGORITHMS_VISUALIZATION_MARCHINGCUBES_H
#define CORE_ALGORITHMS_VISUALIZATION_MARCHINGCUBES_H 1

#include <Core/Datatypes/Legacy/Field/Field.h>
//#include <Core/Datatypes/Matrix.h>

//#include <Core/Thread/Thread.h>
//#include <Core/Geom/GeomGroup.h>

#include <string>
#include <vector>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 #include <Core/Algorithms/Util/AlgoBase.h>
#endif

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRun {
 namespace Core {
  namespace Algorithms {

   class SCISHARE MarchingCubesAlgo : public AlgorithmBase
   {

    public:

    MarchingCubesAlgo();

    static AlgorithmParameterName transparency;
    static AlgorithmParameterName build_geometry;
    static AlgorithmParameterName build_field;
    static AlgorithmParameterName build_node_interpolant;
    static AlgorithmParameterName build_elem_interpolant;
    static AlgorithmParameterName num_threads;

   #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   {
      add_color("color",Color(0.5,0.5,0.5));
      add_colormap("colormap",0);
    };
    #endif

    bool run(FieldHandle input, const std::vector<double>& isovalues);

    bool run(FieldHandle input, const std::vector<double>& isovalues, FieldHandle& field);

    bool run(FieldHandle input, const std::vector<double>& isovalues,
             FieldHandle& field, Datatypes::MatrixHandle& interpolant );

    AlgorithmOutput run(const AlgorithmInput& input) const;

    bool run(FieldHandle input, const std::vector<double>& isovalues,
             FieldHandle& field,
             Datatypes::MatrixHandle& node_interpolant,
             Datatypes::MatrixHandle& elem_interpolant ) const;
   };

  }
 } // End namespace SCIRun
}
#endif

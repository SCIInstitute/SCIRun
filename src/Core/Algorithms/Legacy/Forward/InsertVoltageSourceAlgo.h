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

/*
*  InsertVoltageSource: Insert a voltage source
*
*  Written by:
*   David Weinstein
*   Department of Computer Science
*   University of Utah
*   January 2002
*
*/

#ifndef CORE_ALGORITHMS_LEGACY_FORWARD_INSERTVOLTAGESOURCEALGO_H
#define CORE_ALGORITHMS_LEGACY_FORWARD_INSERTVOLTAGESOURCEALGO_H

#include <Core/Datatypes/Legacy/Field/FieldFwd.h>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Forward/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Forward {

        ALGORITHM_PARAMETER_DECL(InterpolateOutside);
        ALGORITHM_PARAMETER_DECL(GroundFirst);

        class SCISHARE InsertVoltageSourceAlgo //: public AlgorithmBase
        {
        public:
          InsertVoltageSourceAlgo();
          InsertVoltageSourceAlgo(bool groundFirst, bool outside);
          void ExecuteAlgorithm(const FieldHandle& isourceH, FieldHandle& omeshH, Datatypes::DenseMatrixHandle& odirichletMatrix);
          //virtual AlgorithmOutput run_generic(const AlgorithmInput&) const;

        private:
          bool outside_;
          bool groundfirst_;
        };
      }
    }
  }
}

#endif
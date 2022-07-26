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

#ifndef MODULES_PARTICLEINCELL_PIConGPUScalarMeshReader_H
#define MODULES_PARTICLEINCELL_PIConGPUScalarMeshReader_H

#include <Dataflow/Network/Module.h>
#include <Modules/Fields/share.h>

namespace SCIRun         {
namespace Modules        {
namespace ParticleInCell {

    class SCISHARE PIConGPUScalarMeshReader : public SCIRun::Dataflow::Networks::Module,
        public HasNoInputPorts,
        public Has3OutputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag>
            {
            public:
                PIConGPUScalarMeshReader();
                virtual void execute();
                virtual void setStateDefaults();
//                static const Core::Algorithms::AlgorithmOutputName ScalarMesh_value;
//                static const Core::Algorithms::AlgorithmOutputName ScalarMesh_unused1;
//                static const Core::Algorithms::AlgorithmOutputName ScalarMesh_unused2;

                OUTPUT_PORT(0, ScalarMesh_value, Matrix);
                OUTPUT_PORT(1, ScalarMesh_unused1, Matrix);
                OUTPUT_PORT(2, ScalarMesh_unused2, Matrix);

                MODULE_TRAITS_AND_INFO(SCIRun::Modules::ModuleFlags::ModuleHasAlgorithm);
            };
}}}
#endif

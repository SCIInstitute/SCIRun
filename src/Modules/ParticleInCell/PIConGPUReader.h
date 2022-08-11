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

#ifndef MODULES_PARTICLEINCELL_PIConGPUReader_H
#define MODULES_PARTICLEINCELL_PIConGPUReader_H

#include <Modules/Fields/share.h>
#include <Dataflow/Network/Module.h>

using position_t = float;

namespace SCIRun         {
namespace Modules        {
namespace ParticleInCell {

    class SCISHARE PIConGPUReader : public SCIRun::Dataflow::Networks::Module,
        public HasNoInputPorts,
        public Has3OutputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag>
            {
            public:
                PIConGPUReader();
                virtual void execute();
                virtual void setStateDefaults();
                void particleData(int buffer_size, float component_x[], float component_y[], float component_z[], float flat_particle_feature[]);
                void scalarField(std::shared_ptr<float> scalarFieldData_buffer, std::vector<long unsigned int> extent_sFD, double buffer_sFD[]);
                void vectorField(std::vector<long unsigned int> extent_vFD, double XYZ_vec[], std::shared_ptr<float> vFD_component_x, std::shared_ptr<float> vFD_component_y, std::shared_ptr<float> vFD_component_z);

//                OUTPUT_PORT(0, x_coordinates, Matrix);
//                OUTPUT_PORT(1, y_coordinates, Matrix);
//                OUTPUT_PORT(2, z_coordinates, Matrix);

                OUTPUT_PORT(0, Particles, Matrix);
                OUTPUT_PORT(1, ScalarField, Matrix);
                OUTPUT_PORT(2, VectorField, Matrix);

                MODULE_TRAITS_AND_INFO(SCIRun::Modules::ModuleFlags::ModuleHasUIAndAlgorithm);
            };
}}}
#endif

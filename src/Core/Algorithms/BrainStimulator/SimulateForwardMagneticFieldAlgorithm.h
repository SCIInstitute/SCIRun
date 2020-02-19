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


///@file SimulateForwardMagneticField.h
///@brief This module computes the magnetic vector potential for a given mesh.
///
///@author
/// Robert Van Uitert (Mon Aug  4 14:46:51 2003), ported by Moritz Dannhauer (09/17/2014)
///
///@details
///  The modules has four inputs: an electric field distribution (first) for mesh elements with defnied conductivity tensors (second), dipole sources (third)
///  within that mesh and detector locations (fourth) to compute the magnetic field at. All inputs are of Field datatype. The algorithm/module is multi-threaded and
///  outputs the magnetic vector potential and its magnitudes as first and second output.

#ifndef CORE_ALGORITHMS_BRAINSTIMULATOR_SIMULATEFORWARDMAGNETICFIELD_H
#define CORE_ALGORITHMS_BRAINSTIMULATOR_SIMULATEFORWARDMAGNETICFIELD_H 1

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <vector>
#include <Core/Algorithms/BrainStimulator/share.h>

namespace SCIRun {
	namespace Core {
		namespace Algorithms {
			namespace BrainStimulator {

class SCISHARE SimulateForwardMagneticFieldAlgo : public AlgorithmBase
{
  public:

    static AlgorithmInputName ElectricField;
    static AlgorithmInputName ConductivityTensor;
    static AlgorithmInputName DipoleSources;
    static AlgorithmInputName DetectorLocations;
    static AlgorithmOutputName MagneticField;
    static AlgorithmOutputName MagneticFieldMagnitudes;
    boost::tuple<FieldHandle, FieldHandle> run(FieldHandle ElectricField, FieldHandle ConductivityTensors, FieldHandle DipoleSources, FieldHandle DetectorLocations) const;
    virtual AlgorithmOutput run(const AlgorithmInput &) const override;

private:

};
}
}
}
}
#endif

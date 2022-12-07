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


#ifndef CORE_ALGORITHMS_BRAINSTIMULATOR_MODELGENERICCOIL_H
#define CORE_ALGORITHMS_BRAINSTIMULATOR_MODELGENERICCOIL_H 1

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/BrainStimulator/share.h>

#include <string>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace BrainStimulator {

  ALGORITHM_PARAMETER_DECL(Type);
  ALGORITHM_PARAMETER_DECL(FigureOf8CoilShape);
  ALGORITHM_PARAMETER_DECL(Rings);
  ALGORITHM_PARAMETER_DECL(WingsAngle);
  ALGORITHM_PARAMETER_DECL(Current);
  ALGORITHM_PARAMETER_DECL(Radius);
  ALGORITHM_PARAMETER_DECL(InnerRadius);
  ALGORITHM_PARAMETER_DECL(OuterRadius);
  ALGORITHM_PARAMETER_DECL(Distance);
  ALGORITHM_PARAMETER_DECL(Layers);
  ALGORITHM_PARAMETER_DECL(LayerStepSize);
  ALGORITHM_PARAMETER_DECL(LevelOfDetail);

  class SCISHARE ModelTMSCoilAlgorithm : public AlgorithmBase
  {
  public:
    ModelTMSCoilAlgorithm()
    {
      addParameter(Parameters::Type,std::string());
      addParameter(Parameters::FigureOf8CoilShape,2);
      addParameter(Parameters::Current,1.0);
      addParameter(Parameters::Rings,9);
      addParameter(Parameters::WingsAngle,0.0);
      addParameter(Parameters::InnerRadius,26.0);
      addParameter(Parameters::OuterRadius,44.0);
      addParameter(Parameters::Distance,2.0);
      addParameter(Parameters::Layers,1);
      addParameter(Parameters::LayerStepSize,1.0);
      addParameter(Parameters::LevelOfDetail,6);
    }

    struct Args
    {
      double current;
      double coilRadiusInner;
      double coilRadiusOuter;
      double coilDistance;
      double wingsAngle;
      size_t coilLevelDetails;
      size_t coilLayers;
      double coilLayersStep;
      size_t rings;
      int type;
    };

    //! Generate the coil geometry
    AlgorithmOutput run(const AlgorithmInput& input) const override;
  };
}}}}

#endif

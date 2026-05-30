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


#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToTetVolMesh.h>
//#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToIrregularMesh.h>
#include <Core/Algorithms/Legacy/Fields/DomainFields/GetDomainBoundaryAlgo.h>
#include <Core/Algorithms/Legacy/Fields/TransformMesh/ScaleFieldMeshAndData.h>
#include <Core/Algorithms/Legacy/Fields/TransformMesh/TransformMeshWithTransform.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SwapFieldDataWithMatrixEntriesAlgo.h>
#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystemAlgo.h>
#include <Core/Algorithms/DataIO/TextToTriSurfField.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Algorithms/DataIO/WriteMatrix.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/ExtractSimpleIsosurfaceAlgo.h>
#include <boost/functional/factory.hpp>
#include <boost/assign.hpp>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms::Math;
using namespace boost::assign;

/// @todo: add unit test

HardCodedAlgorithmFactory::HardCodedAlgorithmFactory()
{
  addToMakerMap();
  addToMakerMapGenerated();
}

void HardCodedAlgorithmFactory::addToMakerMap()
{
  if (factoryMap_.empty())
  {
    insert(factoryMap_)
      ADD_MODULE_ALGORITHM(SolveLinearSystem, SolveLinearSystemAlgo)
      ADD_MODULE_ALGORITHM(SetupTDCS, SetupTDCSAlgorithm)
      ADD_MODULE_ALGORITHM(ReadMatrix, ReadMatrixAlgorithm)
      ADD_MODULE_ALGORITHM(WriteMatrix, WriteMatrixAlgorithm)
      ADD_MODULE_ALGORITHM(TransformMeshWithTransform, TransformMeshWithTransformAlgo)
			ADD_MODULE_ALGORITHM(SwapFieldDataWithMatrixEntries, SwapFieldDataWithMatrixEntriesAlgo)
      ADD_MODULE_ALGORITHM(ExtractSimpleIsosurface, ExtractSimpleIsosurfaceAlgo)
     ;
  }
}

AlgorithmHandle HardCodedAlgorithmFactory::create(const std::string& moduleName, const AlgorithmCollaborator* algoCollaborator) const
{
  AlgorithmHandle h;

  auto func = factoryMap_.find(moduleName);
  if (func != factoryMap_.end())
    h.reset((func->second.second)());

  //TODO: make a convenience function to copy these for "sub-algorithms"
  if (h && algoCollaborator)
  {
    h->setLogger(algoCollaborator->getLogger());
    h->setUpdaterFunc(algoCollaborator->getUpdaterFunc());
  }

  return h;
}

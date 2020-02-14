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


/// @todo Documentation Modules/Fields/ReportFieldInfo.cc

#include <Core/Datatypes/Scalar.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Modules/Fields/ReportFieldInfo.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Field/ReportFieldInfoAlgorithm.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(ReportFieldInfo, MiscField, SCIRun)

ReportFieldInfo::ReportFieldInfo() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(NumNodes);
  INITIALIZE_PORT(NumElements);
  INITIALIZE_PORT(NumData);
  INITIALIZE_PORT(DataMin);
  INITIALIZE_PORT(DataMax);
  INITIALIZE_PORT(FieldSize);
  INITIALIZE_PORT(FieldCenter);
  INITIALIZE_PORT(Dimensions);
  INITIALIZE_PORT(GeomSize);
}

void ReportFieldInfo::execute()
{
  auto field = getRequiredInput(InputField);

  auto output = algo().run(withInputData((InputField, field)));

  get_state()->setTransientValue("ReportedInfo", output.getTransient());

  auto info = transient_value_cast<SCIRun::Core::Algorithms::Fields::ReportFieldInfoAlgorithm::Outputs>(output.getTransient());
  /// @todo: requires knowledge of algorithm type

  sendOutput(NumNodes, boost::make_shared<Int32>(info.numnodes_));
  sendOutput(NumElements, boost::make_shared<Int32>(info.numelements_));
  sendOutput(NumData, boost::make_shared<Int32>(info.numdata_));
  sendOutput(DataMin, boost::make_shared<Double>(info.dataMin));
  sendOutput(DataMax, boost::make_shared<Double>(info.dataMax));
  sendOutput(FieldSize, boost::make_shared<DenseMatrix>(DenseMatrix::fromPoint(info.size)));
  sendOutput(FieldCenter, boost::make_shared<DenseMatrix>(DenseMatrix::fromPoint(info.center)));
  sendOutput(Dimensions, boost::make_shared<DenseMatrix>(DenseMatrix::fromPoint(info.dims)));
  sendOutput(GeomSize, boost::make_shared<Double>(info.geometricSize));
}

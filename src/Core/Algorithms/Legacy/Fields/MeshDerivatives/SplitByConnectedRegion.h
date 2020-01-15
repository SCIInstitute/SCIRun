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


///
///@file SplitByConnectedRegion
///@brief
/// Splits a domain into separate fields as defined by the input field's connectivity.
///
///@author
/// Moritz Dannhauer (ported from SCIRun4)
///
///@details
/// The module separates mesh elements that are not connected and outputs the first 8 fields (chosen based on element size or ordering) or
/// all sub fields stored as a bundle.

#ifndef CORE_ALGORITHMS_FIELDS_MESHDERIVATIVES_SPLITBYCONNECTEDREGION_H
#define CORE_ALGORITHMS_FIELDS_MESHDERIVATIVES_SPLITBYCONNECTEDREGION_H 1

#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Fields {

class SCISHARE SplitFieldByConnectedRegionAlgo : public AlgorithmBase
{
public:
  SplitFieldByConnectedRegionAlgo();

  static AlgorithmInputName InputField;
  static AlgorithmOutputName OutputField1;
  static AlgorithmOutputName OutputField2;
  static AlgorithmOutputName OutputField3;
  static AlgorithmOutputName OutputField4;
  static AlgorithmOutputName OutputField5;
  static AlgorithmOutputName OutputField6;
  static AlgorithmOutputName OutputField7;
  static AlgorithmOutputName OutputField8;

  static AlgorithmParameterName SortDomainBySize();
  static AlgorithmParameterName SortAscending();
  std::vector<FieldHandle> run(FieldHandle input) const;

  AlgorithmOutput run(const AlgorithmInput& input) const;
};

}}}}

#endif

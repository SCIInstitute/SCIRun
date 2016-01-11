/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#ifndef ALGORITHMS_BASE_ALGORITHMMACROS_H
#define ALGORITHMS_BASE_ALGORITHMMACROS_H

#include <Core/Algorithms/Base/AlgorithmInputBuilder.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {

      template <typename T>
      boost::shared_ptr<T> optionalAlgoInput(boost::optional<boost::shared_ptr<T>> opt)
      {
        return opt.get_value_or(nullptr);
      }

    }}}


#define make_input(list) SCIRun::Core::Algorithms::AlgoInputBuilder() list .build()
#define withInputData(list) make_input(list)
#define make_output(portName) SCIRun::Core::Algorithms::AlgorithmParameterName(#portName)
#define get_output(outputObj, portName, type) boost::dynamic_pointer_cast<type>(outputObj[make_output(portName)]);
#define ALGORITHM_PARAMETER_DECL(name) namespace Parameters { SCISHARE extern SCIRun::Core::Algorithms::AlgorithmParameterName _init_##name(); static const SCIRun::Core::Algorithms::AlgorithmParameterName& name(_init_##name()); }
#define ALGORITHM_PARAMETER_DEF(ns, name) SCIRun::Core::Algorithms::AlgorithmParameterName SCIRun::Core::Algorithms::ns::Parameters::_init_##name() { return SCIRun::Core::Algorithms::AlgorithmParameterName(#name); }

#endif

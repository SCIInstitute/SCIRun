/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2026 Scientific Computing and Imaging Institute,
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


#ifndef CORE_SERIALIZATION_NETWORK_NETWORK_TO_PYTHON_H
#define CORE_SERIALIZATION_NETWORK_NETWORK_TO_PYTHON_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Algorithms/Base/Variable.h>
#include <optional>
#include <string>
#include <Dataflow/Serialization/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  /// Writes a python script that rebuilds the network through the scirun_* API (#1671).
  /// Port indices come from the live network; layout (positions, disabled connections)
  /// comes from the saved file, which may be null.
  SCISHARE std::string networkToPythonScript(const NetworkStateInterface& network,
    const NetworkFile* layout, const std::string& sourceName = "");

  /// A python literal for a module state value, or nothing if it has no python form.
  SCISHARE std::optional<std::string> pythonLiteral(const Core::Algorithms::Variable& var);

}}}

#endif

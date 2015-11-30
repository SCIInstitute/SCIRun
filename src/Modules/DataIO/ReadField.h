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

/// @todo Documentation Module/DataIO/ReadField.h

#ifndef MODULES_DATAIO_READ_FIELD_H
#define MODULES_DATAIO_READ_FIELD_H

#include <Core/Datatypes/Mesh/FieldFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Modules/DataIO/GenericReader.h>
#include <Modules/DataIO/share.h>

namespace SCIRun {
namespace Modules {
namespace DataIO {

  class SCISHARE ReadFieldModule : public GenericReader<FieldHandle, FieldPortTag>
  {
  public:
    typedef GenericReader<FieldHandle, FieldPortTag> my_base;
    ReadFieldModule();
    virtual void execute() override;
    virtual bool useCustomImporter(const std::string& filename) const override;
    virtual bool call_importer(const std::string& filename, FieldHandle& handle) override;

    OUTPUT_PORT(0, Field, LegacyField);

    static std::string fileTypeList();
  };

}}}

#endif

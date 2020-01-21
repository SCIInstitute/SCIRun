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


#ifndef MODULES_DATAIO_READ_BUNDLE_H
#define MODULES_DATAIO_READ_BUNDLE_H

#include <Modules/DataIO/GenericReader.h>
#include <Modules/DataIO/share.h>

namespace SCIRun {
namespace Modules {
namespace DataIO {

  class SCISHARE ReadBundle : public GenericReader<Core::Datatypes::BundleHandle, BundlePortTag>
  {
  public:
    typedef GenericReader<Core::Datatypes::BundleHandle, BundlePortTag> my_base;
    ReadBundle();
    virtual void execute() override;
    virtual bool useCustomImporter(const std::string& filename) const override { return false; }

    OUTPUT_PORT(0, Bundle, Bundle);

    static std::string fileTypeList();

    MODULE_TRAITS_AND_INFO(ModuleHasUI)

  protected:
    virtual std::string defaultFileTypeName() const override;
  };

}}}

#endif

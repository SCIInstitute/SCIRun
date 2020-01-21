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


/// @todo Documentation Modules/DataIO/WriteMatrix.h

#ifndef MODULES_DATAIO_WRITE_MATRIX_H
#define MODULES_DATAIO_WRITE_MATRIX_H

#include <Modules/DataIO/GenericWriter.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Modules/DataIO/share.h>

namespace SCIRun {
  namespace Modules {
    namespace DataIO {

      class SCISHARE WriteMatrix : public GenericWriter<Core::Datatypes::MatrixHandle, MatrixPortTag>
      {
      public:
        typedef GenericWriter<Core::Datatypes::MatrixHandle, MatrixPortTag> my_base;
        WriteMatrix();
        virtual void execute() override;
        virtual bool useCustomExporter(const std::string& filename) const override;
        virtual bool call_exporter(const std::string& filename) override;

        INPUT_PORT(0, MatrixToWrite, Matrix);

        MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)

      protected:
        virtual std::string defaultFileTypeName() const override;
      };

    }}}

#endif

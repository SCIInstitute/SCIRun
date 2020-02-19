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


/// @todo Documentation Modules/DataIO/WriteG3D.h

#ifndef MODULES_DATAIO_WRITE_G3D_H
#define MODULES_DATAIO_WRITE_G3D_H

#include <Core/Datatypes/Mesh/FieldFwd.h>
#include <Modules/DataIO/GenericWriter.h>
#include <Core/Thread/Interruptible.h>
#include <Core/Datatypes/Color.h>
#include <Modules/DataIO/share.h>

namespace SCIRun {
  namespace Modules {
    namespace DataIO {

      class SCISHARE WriteG3D : public GenericWriter<FieldHandle, FieldPortTag>,
        //public GenericWriter<FieldHandle, FieldPortTag, Has2InputPorts<FieldPortTag, ColorMapPortTag>>,
        public Core::Thread::Interruptible
      {
      public:
        typedef GenericWriter<FieldHandle, FieldPortTag> my_base;
        //typedef GenericWriter<FieldHandle, FieldPortTag, Has2InputPorts<FieldPortTag, ColorMapPortTag>> my_base;
        WriteG3D();
        virtual void execute() override;
        virtual bool useCustomExporter(const std::string& filename) const override;
        virtual bool call_exporter(const std::string& filename) override;

        static const Core::Algorithms::AlgorithmParameterName EnableTransparency;
        static const Core::Algorithms::AlgorithmParameterName TransparencyValue;
        static const Core::Algorithms::AlgorithmParameterName Coloring;
        static const Core::Algorithms::AlgorithmParameterName DefaultColor;

        INPUT_PORT(0, FieldToWrite, Field);
        //INPUT_PORT(1, ColorMapObject, ColorMap);

        virtual void setStateDefaults() override;

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
      protected:
        virtual std::string defaultFileTypeName() const override;

      private:
        void calculateColors();

        std::vector<Core::Datatypes::ColorRGB> colors_;
      };

    }}}

#endif

/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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

#ifndef MODULES_VISUALIZATION_CREATETESTINGARROW_H
#define MODULES_VISUALIZATION_CREATETESTINGARROW_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Modules/Visualization/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Visualization
      {
        ALGORITHM_PARAMETER_DECL(XLocation);
        ALGORITHM_PARAMETER_DECL(YLocation);
        ALGORITHM_PARAMETER_DECL(ZLocation);
      }
    }
  }
  namespace Modules
  {
    namespace Visualization
    {
      class CreateTestingArrowImpl;

      class SCISHARE CreateTestingArrow : public Dataflow::Networks::GeometryGeneratingModule,
        public HasNoInputPorts,
        public Has2OutputPorts<GeometryPortTag, FieldPortTag>
      {
      public:
        CreateTestingArrow();
        void execute() override;
        OUTPUT_PORT(0, Arrow, GeometryObject);
        OUTPUT_PORT(1, GeneratedPoint, Field);

        void setStateDefaults() override;

        MODULE_TRAITS_AND_INFO(NoAlgoOrUI)
      private:
        void processWidgetFeedback(const Core::Datatypes::ModuleFeedback& var);
        void adjustGeometryFromTransform(const Core::Geometry::Transform& transformMatrix);

        boost::shared_ptr<CreateTestingArrowImpl> impl_;
      };
    }
  }
}

#endif

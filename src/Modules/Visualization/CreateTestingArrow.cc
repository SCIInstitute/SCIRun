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

#include <Modules/Visualization/CreateTestingArrow.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Graphics/Widgets/WidgetFactory.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Graphics::Datatypes;
using namespace Core::Datatypes;
using namespace Dataflow::Networks;
using namespace Core::Geometry;

namespace SCIRun
{
  namespace Modules
  {
    namespace Visualization
    {
      class CreateTestingArrowImpl
      {
      public:
        Transform userWidgetTransform_;
        BBox box_initial_bounds_;
        BBox bbox_;
        Point origin_ {0,0,0};
      };
    }
  }
}

MODULE_INFO_DEF(CreateTestingArrow, Visualization, SCIRun)

CreateTestingArrow::CreateTestingArrow() : GeometryGeneratingModule(staticInfo_), impl_(new CreateTestingArrowImpl)
{
  INITIALIZE_PORT(Arrow);
}

void CreateTestingArrow::setStateDefaults()
{
  getOutputPort(Arrow)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void CreateTestingArrow::execute()
{
  //std::cout << __FILE__ << __LINE__ << " " << impl_->origin_ << std::endl;
  CommonWidgetParameters common
  {
    1.0, "red", impl_->origin_,
    {impl_->origin_, Point(impl_->origin_ + Point{1,1,1})},
    10
  };
  ArrowParameters arrowParams
  {
    common,
    impl_->origin_, Vector{1,1,1}, true, 2, 4
  };
  auto arrow = WidgetFactory::createArrowWidget(
    {*this, "testArrow1"},
    arrowParams
  );
  sendOutput(Arrow, arrow);
}

void CreateTestingArrow::processWidgetFeedback(const ModuleFeedback& var)
{
  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (vsf.matchesWithModuleId(id()) && impl_->userWidgetTransform_ != vsf.transform)
    {
      adjustGeometryFromTransform(vsf.transform);
      enqueueExecuteAgain(false);
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void CreateTestingArrow::adjustGeometryFromTransform(const Transform& transformMatrix)
{
  impl_->origin_ = transformMatrix * impl_->origin_;
  impl_->userWidgetTransform_ = transformMatrix;
}
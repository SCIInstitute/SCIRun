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
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Graphics/Widgets/WidgetFactory.h>
#include <Graphics/Widgets/ArrowWidget.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Graphics::Datatypes;
using namespace Core::Datatypes;
using namespace Core::Algorithms::Visualization;
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

        Point currentLocation(ModuleStateHandle state) const;
      };
    }
  }
}

ALGORITHM_PARAMETER_DEF(Visualization, XLocation);
ALGORITHM_PARAMETER_DEF(Visualization, YLocation);
ALGORITHM_PARAMETER_DEF(Visualization, ZLocation);

MODULE_INFO_DEF(CreateTestingArrow, Visualization, SCIRun)

CreateTestingArrow::CreateTestingArrow() : GeometryGeneratingModule(staticInfo_), impl_(new CreateTestingArrowImpl)
{
  INITIALIZE_PORT(Arrow);
  INITIALIZE_PORT(GeneratedPoint);
}

void CreateTestingArrow::setStateDefaults()
{
  auto state = get_state();
  using namespace Parameters;
  state->setValue(XLocation, 0.0);
  state->setValue(YLocation, 0.0);
  state->setValue(ZLocation, 0.0);

  getOutputPort(Arrow)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void CreateTestingArrow::execute()
{
  impl_->origin_ = impl_->currentLocation(get_state());
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
  //std::cout << __FILE__ << __LINE__ << " " << (*(dynamic_cast<ArrowWidget*>(arrow.get())->subwidgetBegin()))->origin() << std::endl;
  sendOutput(Arrow, arrow);

  {
    FieldInformation fi("PointCloudMesh", 0, "double");
    auto mesh = CreateMesh(fi);
    mesh->vmesh()->add_point(impl_->origin_);
    auto ofield = CreateField(fi, mesh);
    ofield->vfield()->resize_values();
    sendOutput(GeneratedPoint, ofield);
  }
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

Point CreateTestingArrowImpl::currentLocation(ModuleStateHandle state) const
{
  using namespace Parameters;
  return Point(state->getValue(XLocation).toDouble(), state->getValue(YLocation).toDouble(), state->getValue(ZLocation).toDouble());
}

void CreateTestingArrow::adjustGeometryFromTransform(const Transform& transformMatrix)
{
  impl_->origin_ = transformMatrix * impl_->origin_;
  auto state = get_state();
  using namespace Parameters;
  state->setValue(XLocation, impl_->origin_.x());
  state->setValue(YLocation, impl_->origin_.y());
  state->setValue(ZLocation, impl_->origin_.z());
  impl_->userWidgetTransform_ = transformMatrix;
}

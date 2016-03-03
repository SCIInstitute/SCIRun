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

#ifndef MODULES_RENDER_VIEWSCENE_H
#define MODULES_RENDER_VIEWSCENE_H

#include <Dataflow/Network/Module.h>
#include <Core/Thread/Mutex.h>
#include <Core/Algorithms/Base/AlgorithmMacros.h>
#include <Modules/Render/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Render
      {
        ALGORITHM_PARAMETER_DECL(GeomData);
        ALGORITHM_PARAMETER_DECL(GeometryFeedbackInfo);
        ALGORITHM_PARAMETER_DECL(ScreenshotData);
      }
    }
  }

namespace Modules {
namespace Render {

/// @class ViewScene
/// @brief The ViewScene displays interactive graphical output to the computer screen.
///
/// Use the ViewScene to see a geometry, or spatial data. The ViewScene
/// provides access to many simulation parameters and controls, thus,
/// indirectly initiates new iterations of the simulation steps important to
/// computational steering.

  class SCISHARE ViewScene : public Dataflow::Networks::ModuleWithAsyncDynamicPorts,
    public Has1InputPort<AsyncDynamicPortTag<GeometryPortTag>>,
#ifdef BUILD_TESTING
    public Has1OutputPort<MatrixPortTag>
#else
    public HasNoOutputPorts
#endif
  {
  public:
    ViewScene();
    virtual void asyncExecute(const Dataflow::Networks::PortId& pid, Core::Datatypes::DatatypeHandle data) override;
    virtual void setStateDefaults() override;

    static Dataflow::Networks::ModuleLookupInfo staticInfo_;
    static Core::Algorithms::AlgorithmParameterName BackgroundColor;
    static Core::Algorithms::AlgorithmParameterName ShowScaleBar;
    static Core::Algorithms::AlgorithmParameterName ScaleBarUnitValue;
    static Core::Algorithms::AlgorithmParameterName ScaleBarLength;
    static Core::Algorithms::AlgorithmParameterName ScaleBarHeight;
    static Core::Algorithms::AlgorithmParameterName ScaleBarMultiplier;
    static Core::Algorithms::AlgorithmParameterName ScaleBarNumTicks;
    static Core::Algorithms::AlgorithmParameterName ScaleBarLineWidth;
    static Core::Algorithms::AlgorithmParameterName ScaleBarFontSize;

    INPUT_PORT_DYNAMIC(0, GeneralGeom, GeometryObject);
#ifdef BUILD_TESTING
    OUTPUT_PORT(0, ScreenshotData, DenseMatrix);
    virtual void execute() override;
#endif

    static Core::Thread::Mutex mutex_;

    typedef std::set<Core::Datatypes::GeometryBaseHandle> GeomList;
    typedef boost::shared_ptr<GeomList> GeomListPtr;
    typedef std::map<Dataflow::Networks::PortId, Core::Datatypes::GeometryBaseHandle> ActiveGeometryMap;
  protected:
    virtual void portRemovedSlotImpl(const Dataflow::Networks::PortId& pid) override;
    virtual void postStateChangeInternalSignalHookup() override;
  private:
    void processViewSceneObjectFeedback();
    void updateTransientList();
    ActiveGeometryMap activeGeoms_;
  };
}}}

#endif

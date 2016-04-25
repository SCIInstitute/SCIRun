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

#ifndef INTERFACE_MODULES_VIEW_SCENE_H
#define INTERFACE_MODULES_VIEW_SCENE_H

/// \todo Make this definition specific to windows.
#define NOMINMAX

#include "Interface/Modules/Render/ui_ViewScene.h"

#include <boost/shared_ptr.hpp>

#include <Modules/Basic/SendScalarModuleState.h>
#include <Modules/Visualization/TextBuilder.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Interface/Modules/Render/ViewSceneControlsDock.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Interface/Modules/Render/share.h>

//TODO: needs to inherit from ModuleWidget somehow
class QToolBar;
class QStandardItemModel;
class QStandardItem;
class QGLWidget;

namespace SCIRun {

  namespace Render { class SRInterface; }

  namespace Gui {

    class GLWidget;
    class ViewSceneControlsDock;


    class SCISHARE ViewSceneDialog : public ModuleDialogGeneric,
      public Ui::ViewScene
    {
      Q_OBJECT

    public:
      ViewSceneDialog(const std::string& name,
        Dataflow::Networks::ModuleStateHandle state,
        QWidget* parent = nullptr);

    Q_SIGNALS:
      void newGeometryValueForwarder();
      void mousePressSignalForTestingGeometryObjectFeedback(int x, int y);

      protected Q_SLOTS:
      void menuMouseControlChanged(int index);
      void autoViewClicked();
      void newGeometryValue();
      void newOwnGeometryValue();
      void autoViewOnLoadChecked(bool value);
      void useOrthoViewChecked(bool value);
      void showOrientationChecked(bool value);
      void showAxisChecked(bool value);
      void viewBarButtonClicked();
      void viewAxisSelected(int index);
      void viewVectorSelected(int index);
      void configurationButtonClicked();
      void assignBackgroundColor();
      void setTransparencySortTypeContinuous(bool index);
      void setTransparencySortTypeUpdate(bool index);
      void setTransparencySortTypeLists(bool index);
      void handleUnselectedItem(const QString& name);
      void handleSelectedItem(const QString& name);
      void selectAllClicked();
      void deselectAllClicked();
      void adjustZoomSpeed(int value);
      void invertZoomClicked(bool value);
      void screenshotClicked();
      void saveNewGeometryChanged(int state);
      void sendGeometryFeedbackToState(int x, int y);
      //Clipping Plane 
      void setClippingPlaneIndex(int index);
      void setClippingPlaneVisible(bool value);
      void setClippingPlaneFrameOn(bool value);
      void reverseClippingPlaneNormal(bool value);
      void setClippingPlaneX(int index);
      void setClippingPlaneY(int index);
      void setClippingPlaneZ(int index);
      void setClippingPlaneD(int index);
      //Materials Controls
      void setAmbientValue(double value);
      void setDiffuseValue(double value);
      void setSpecularValue(double value);
      void setShininessValue(double value);
      void setEmissionValue(double value);
      void setFogOn(bool value);
      void setFogOnVisibleObjects(bool value);
      void setFogUseBGColor(bool value);
      void setFogStartValue(double value);
      void setFogEndValue(double value);
      void assignFogColor();
      //Scale Bar
      void setScaleBarVisible(bool value);
      void setScaleBarFontSize(int value);
      void setScaleBarUnitValue(const QString& text);
      void setScaleBarLength(double value);
      void setScaleBarHeight(double value);
      void setScaleBarMultiplier(double value);
      void setScaleBarNumTicks(int value);
      void setScaleBarLineWidth(double value);
      void setScaleBar();
      //Render Settings
      void lightingChecked(bool value);
      void showBBoxChecked(bool value);
      void useClipChecked(bool value);
      void stereoChecked(bool value);
      void useBackCullChecked(bool value);
      void displayListChecked(bool value);
      void setStereoFusion(int value);
      void setPolygonOffset(int value);
      void setTextOffset(int value);
      void setFieldOfView(int value);

    protected:
      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void wheelEvent(QWheelEvent* event);
      virtual void keyPressEvent(QKeyEvent* event);
      virtual void keyReleaseEvent(QKeyEvent*event);
      virtual void closeEvent(QCloseEvent* evt) override;
      virtual void showEvent(QShowEvent* evt) override;
      virtual void hideEvent(QHideEvent* evt) override;
      virtual void contextMenuEvent(QContextMenuEvent* evt) override {}
    private:
      struct ClippingPlane {
        bool visible, showFrame, reverseNormal;
        double x, y, z, d;
      };

      struct ScaleBar {
        bool visible;
        int fontSize;
        double length, height, multiplier, numTicks, lineWidth;
        std::string unit;
        double projLength;
      };

      void selectObject(const int x, const int y);
      void restoreObjColor();
      void updatClippingPlaneDisplay();
      bool isObjectUnselected(const std::string& name);
      void addToolBar();
      void addAutoViewButton();
      void addScreenshotButton();
      void addViewBarButton();
      void addViewBar();
      void addViewOptions();
      void addConfigurationButton();
      void addConfigurationDock(const QString& viewName);
      void setupClippingPlanes();
      void setupMaterials();
      void setupScaleBar();
      void setupRenderTabValues();
      void hideConfigurationDock();
      void takeScreenshot();
      void sendScreenshotDownstreamForTesting();

      void lookDownAxisX(int upIndex, glm::vec3& up);
      void lookDownAxisY(int upIndex, glm::vec3& up);
      void lookDownAxisZ(int upIndex, glm::vec3& up);

      // update scale bar geometries
      Graphics::Datatypes::GeometryHandle buildGeometryScaleBar();
      void updateScaleBarLength();

      // update clipping plane geometries
      void buildGeomClippingPlanes();
      void buildGeometryClippingPlane(int index, glm::vec4 plane, const Core::Geometry::BBox& bbox);

      //set material
      void setMaterialFactor(int factor, double value);

      //set fog
      void setFog(int factor, double value);
      void setFogColor(const glm::vec4 &color);

      GLWidget*                             mGLWidget;            ///< GL widget containing context.
      std::weak_ptr<Render::SRInterface>    mSpire;               ///< Instance of Spire.
      QToolBar*                             mToolBar;             ///< Tool bar.
      QToolBar*                             mViewBar;             ///< Tool bar for view options.
      QComboBox*                            mDownViewBox;         ///< Combo box for Down axis options.
      QComboBox*                            mUpVectorBox;         ///< Combo box for Up Vector options.
      ViewSceneControlsDock*                mConfigurationDock;   ///< Dock holding configuration functions

      int counter_;
      bool shown_;
      bool hideViewBar_;
      bool showConfiguration_;
      bool itemValueChanged_;
      bool invertZoom_;
      bool shiftdown_;
      bool selected_;
      int clippingPlaneIndex_;
      QColor bgColor_;
      QColor fogColor_;
      ScaleBar scaleBar_;
      std::vector<ClippingPlane> clippingPlanes_;
      std::vector<std::string> unselectedObjectNames_;
      std::vector<std::string> previousObjectNames_;
      class Screenshot* screenshotTaker_;
      bool saveScreenshotOnNewGeometry_;

      //geometries
      Modules::Visualization::TextBuilder textBuilder_;
      Graphics::Datatypes::GeometryHandle scaleBarGeom_;
      std::vector<Graphics::Datatypes::GeometryHandle> clippingPlaneGeoms_;

      friend class ViewSceneControlsDock;

      std::unique_ptr<Core::GeometryIDGenerator> gid_;
    };
  }
}

#endif

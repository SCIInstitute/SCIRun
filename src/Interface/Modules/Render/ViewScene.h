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

      std::string toString(std::string prefix) const;
      void adjustToolbar() override;

    Q_SIGNALS:
      void newGeometryValueForwarder();
      void mousePressSignalForTestingGeometryObjectFeedback(int x, int y, const std::string& selName);

    protected Q_SLOTS:
      void menuMouseControlChanged(int index);
      void autoViewClicked();
      void newGeometryValue();
      void autoViewOnLoadChecked(bool value);
      void useOrthoViewChecked(bool value);
      void showOrientationChecked(bool value);
      void setOrientAxisSize(int value);
      void setOrientAxisPosX(int pos);
      void setOrientAxisPosY(int pos);
      void setCenterOrientPos();
      void setDefaultOrientPos();
      void showAxisChecked(bool value);
      void viewBarButtonClicked();
      void viewAxisSelected(const QString& name);
      void viewVectorSelected(const QString& name);
      void configurationButtonClicked();
      void assignBackgroundColor();
      void setTransparencySortTypeContinuous(bool index);
      void setTransparencySortTypeUpdate(bool index);
      void setTransparencySortTypeLists(bool index);
      void adjustZoomSpeed(int value);
      void invertZoomClicked(bool value);
      void screenshotClicked();
      void saveNewGeometryChanged(int state);
      void sendGeometryFeedbackToState(int x, int y, const std::string& selName);
      void updateMeshComponentSelection(const QString& moduleId, const QString& component, bool selected);

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
      void setLightPosition(int index);
      void setLightColor(int index);
      void toggleHeadLight(bool value);
      void toggleLight1(bool value);
      void toggleLight2(bool value);
      void toggleLight3(bool value);
      void resizingDone();

      void lockRotationToggled();
      void lockPanningToggled();
      void lockZoomToggled();
      void lockAllTriggered();
      void unlockAllTriggered();
      void toggleLockColor(bool locked);

    protected:
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent*event) override;
      void closeEvent(QCloseEvent* evt) override;
      void showEvent(QShowEvent* evt) override;
      void hideEvent(QHideEvent* evt) override;
      void contextMenuEvent(QContextMenuEvent* evt) override {}
      void resizeEvent(QResizeEvent *event) override;

      void pullSpecial() override;

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

      void addToolBar();
      void setupClippingPlanes();
      void setupScaleBar();
      void setInitialLightValues();
      void setupMaterials();
      void setupRenderTabValues();//?why isnt this called?

      void addAutoViewButton();
      void addScreenshotButton();
      void addViewBarButton();
      void addControlLockButton();
      void addToolbarButton(QPushButton* button);
      void addViewBar();
      void addViewOptions();
      void addConfigurationButton();
      void addConfigurationDock();

      QColor checkColorSetting(std::string& rgb, QColor defaultColor);
      void selectObject(const int x, const int y);
      std::string restoreObjColor();
      void updatClippingPlaneDisplay();

      void takeScreenshot();
      void sendScreenshotDownstreamForTesting();

      void toggleLightOnOff(int index, bool value);

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

      GLWidget*                             mGLWidget                     {};         ///< GL widget containing context.
      std::weak_ptr<Render::SRInterface>    mSpire                        {};         ///< Instance of Spire.
      QToolBar*                             mToolBar                      {};         ///< Tool bar.
      QToolBar*                             mViewBar                      {};         ///< Tool bar for view options.
      QComboBox*                            mDownViewBox                  {};         ///< Combo box for Down axis options.
      QComboBox*                            mUpVectorBox                  {};         ///< Combo box for Up Vector options.
      ViewSceneControlsDock*                mConfigurationDock            {nullptr};  ///< Dock holding configuration functions

      bool                                  shown_                        {false};
      bool                                  hideViewBar_                  {};
      bool                                  invertZoom_                   {};
      bool                                  shiftdown_                    {false};
      bool                                  selected_                     {false};
      int                                   clippingPlaneIndex_           {0};
      QColor                                bgColor_                      {};
      QColor                                fogColor_                     {};
      ScaleBar                              scaleBar_                     {};
      std::vector<ClippingPlane>            clippingPlanes_               {};
      class Screenshot*                     screenshotTaker_              {nullptr};
      bool                                  saveScreenshotOnNewGeometry_  {false};
      bool                                  pulledSavedVisibility_        {false};
      QTimer                                resizeTimer_                  {};

      //geometries
      Modules::Visualization::TextBuilder               textBuilder_        {};
      Graphics::Datatypes::GeometryHandle               scaleBarGeom_       {};
      std::vector<Graphics::Datatypes::GeometryHandle>  clippingPlaneGeoms_ {};
      QAction*                                          lockRotation_       {};
      QAction*                                          lockPan_            {};
      QAction*                                          lockZoom_           {};
      QPushButton*                                      controlLock_        {};
      QPushButton*                                      autoViewButton_     {};
      QPushButton*                                      viewBarBtn_         {};

      friend class ViewSceneControlsDock;

      std::unique_ptr<Core::GeometryIDGenerator> gid_;
    };

  } // namespace Gui
} // namespace SCIRun

#endif

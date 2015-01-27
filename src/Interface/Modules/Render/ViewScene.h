/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
#include <Interface/Modules/Base/ModuleDialogGeneric.h>

#include <Interface/Modules/Render/ViewSceneControlsDock.h>
#include <Interface/Modules/Render/namespaces.h>
#include <Interface/Modules/Render/share.h>

#include <glm/glm.hpp>

//TODO: needs to inherit from ModuleWidget somehow
class QToolBar;
class QStandardItemModel;
class QStandardItem;

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
        SCIRun::Dataflow::Networks::ModuleStateHandle state,
        QWidget* parent = 0);
      virtual void pull() {}

    Q_SIGNALS:
      void newGeometryValueForwarder();

      protected Q_SLOTS:
      void menuMouseControlChanged(int index);
      void autoViewClicked();
      void newGeometryValue();
      void showOrientationChecked(bool value);
      void viewBarButtonClicked();
      void viewAxisSelected(int index);
      void viewVectorSelected(int index);
      void configurationButtonClicked();
      void assignBackgroundColor();

    protected:
      virtual void closeEvent(QCloseEvent* evt) override;
      virtual void showEvent(QShowEvent* evt) override;
      virtual void hideEvent(QHideEvent* evt) override;
      virtual void contextMenuEvent(QContextMenuEvent* evt) override {}
    private:
      void addToolBar();
      void addAutoViewButton();
      void addObjectToggleMenu();
      void addViewBarButton();
      void addViewBar();
      void addViewOptions();
      void addConfigurationButton();
      void addConfigurationDock(const QString& viewName);
      void hideConfigurationDock();

      void lookDownAxisX(int upIndex, glm::vec3& up);
      void lookDownAxisY(int upIndex, glm::vec3& up);
      void lookDownAxisZ(int upIndex, glm::vec3& up);

      GLWidget*                             mGLWidget;            ///< GL widget containing context.
      std::weak_ptr<Render::SRInterface>    mSpire;               ///< Instance of Spire.
      QToolBar*                             mToolBar;             ///< Tool bar.
      QToolBar*                             mViewBar;             ///< Tool bar for view options.
      QComboBox*                            mDownViewBox;         ///< Combo box for Down axis options.
      QComboBox*                            mUpVectorBox;         ///< Combo box for Up Vector options.
      ViewSceneControlsDock*                mConfigurationDock;   ///< Dock holding configuration functions               mConfigurationDialog; ///< Menu holding configuration functions

      bool shown_;
      bool hideViewBar_;
      bool showConfiguration_;
      std::shared_ptr<class ViewSceneItemManager> itemManager_;

      friend class ViewSceneControlsDock;
		};

		class ViewSceneItemManager : public QObject
		{
			Q_OBJECT
		public:
			ViewSceneItemManager();
			QStandardItemModel* model() { return model_; }
			public Q_SLOTS:
			void addItem(const QString& name);
			void removeItem(const QString& name);
			void removeAll();
		Q_SIGNALS:
			void itemSelected(const QString& name);
			void itemUnselected(const QString& name);
			private Q_SLOTS:
			void slotChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
		private:
			QStandardItemModel* model_;
			std::vector<QStandardItem*> items_;
		};

	}
}

#endif

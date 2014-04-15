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

#include <spire/Interface.h>
#include <Interface/Modules/Render/namespaces.h>

#include <Interface/Modules/Render/SpireSCIRun/SRInterface.h>
#include <Interface/Modules/Render/SpireSCIRun/SRCommonAttributes.h>
#include <Interface/Modules/Render/SpireSCIRun/SRCommonUniforms.h>

#include <Interface/Modules/Render/GLWidget.h>

#include <Interface/Modules/Render/share.h>

//TODO: needs to inherit from ModuleWidget somehow
class QToolBar;
class QStandardItemModel;
class QStandardItem;

namespace SCIRun {
namespace Gui {

class SCISHARE ViewSceneDialog : public ModuleDialogGeneric, 
  public Ui::ViewScene
{
	Q_OBJECT
	
public:
  ViewSceneDialog(const std::string& name, 
    SCIRun::Dataflow::Networks::ModuleStateHandle state,
    QWidget* parent = 0);
  ~ViewSceneDialog();
  virtual void pull() {}

  virtual void moduleExecuted();

protected Q_SLOTS:
  void menuMouseControlChanged(int index);
  void autoViewClicked();

protected:
  virtual void closeEvent(QCloseEvent *evt) override;
  virtual void showEvent(QShowEvent* event) override;
private:
  void addToolBar();
  void addMouseMenu();
  void addAutoViewButton();
  void addObjectToggleMenu();

  GLWidget*                     mGLWidget;  ///< GL widget containing context.
  std::weak_ptr<SRInterface>    mSpire;     ///< Instance of Spire.
  QToolBar*                     mToolBar;   ///< Tool bar.
  bool shown_;
  std::shared_ptr<class ViewSceneItemManager> itemManager_;
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

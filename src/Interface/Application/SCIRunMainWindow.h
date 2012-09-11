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

#ifndef INTERFACE_APPLICATION_SCIRUN_MAIN_WINDOW_H
#define INTERFACE_APPLICATION_SCIRUN_MAIN_WINDOW_H

#include <QDialog>
#include <boost/shared_ptr.hpp>
#include "ui_SCIRunMainWindow.h"

namespace SCIRun {
  namespace Dataflow {
    namespace Engine {
      class NetworkEditorController;
    }}}

namespace SCIRun {
namespace Gui {

//#ifdef BUILD_VTK_SUPPORT
//class RenderWindow;
//#endif

class NetworkEditor;

class SCIRunMainWindow : public QMainWindow, public Ui::SCIRunMainWindow
{
	Q_OBJECT
public:
	static SCIRunMainWindow* Instance();
  void setController(boost::shared_ptr<SCIRun::Dataflow::Engine::NetworkEditorController> controller);
protected Q_SLOTS:
  void ToggleRenderer();
protected:
  virtual void closeEvent(QCloseEvent* event);
private:
  static SCIRunMainWindow* instance_;
  SCIRunMainWindow();
  NetworkEditor* networkEditor_;
  
//#ifdef BUILD_VTK_SUPPORT
//  RenderWindow* renderWindow_;
//#endif

private:
  bool okToContinue();
  void saveNetworkFile(const QString& fileName);
  void setCurrentFile(const QString& fileName);
  void updateRecentFileActions();
  QString strippedName(const QString& fillFileName);
  QString currentFile_;
private Q_SLOTS:
  void saveNetworkAs();
  void saveNetwork();
  void loadNetwork();
  bool clearNetwork();
  void networkModified();
};

}
}
#endif

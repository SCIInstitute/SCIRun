/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <QDialog>
#include <vtkSmartPointer.h>
#include <vtkVector.h>
#include <Core/Dataflow/Network/RendererInterface.h>

class QVTKWidget;
class vtkRenderer;
class vtkArrowSource;
class vtkPolyDataMapper;
class vtkMatrix4x4;
class vtkTransform;
class vtkTextActor;

namespace Ui {
class RenderWindow;
}

namespace SCIRun {
namespace Gui {

class RenderWindow : public QDialog, public SCIRun::Domain::Networks::RendererInterface
{
  Q_OBJECT
    
public:
  explicit RenderWindow(QWidget *parent = 0);
  ~RenderWindow();

  /// Hack: Accepts a 6xn matrix whose format is as follows.
  ///  rows 1..3 - direction of vectors.
  ///  rows 4..6 - position of vectors in 3D space.
  virtual void setVectorField(const SCIRun::Domain::Datatypes::DenseMatrixGeneric<double>& m);

  /// Change default text output
  virtual void setText(const char* text);

  /// Clear the scene of all actors.
  virtual void clearScene();
    
private:

  /// Sets up a default static vector field.
  void setupDefaultVectorField();

  /// Sets up a helix vector field.
  void setupHelixVectorField();

  /// This function constructs a 3x3 rotation matrix from an 'at' vector.
  /// While we have no control over the rotation about the given vector,
  /// in many cases, we don't care. If you want fine grained control of the
  /// rotation, two vectors must be provided and their cross product calculated.
  /// A 4x4 homogeneous vtk matrix is returned instead of a 3x3 rotation.
  /// The last column is (0,0,0,1)
  static vtkSmartPointer<vtkTransform> matFromVec(const vtkVector3d& v,
                                                  const vtkVector3d& pos);
  /// Same as above, but uses the input vector as the 'x' instead of the 'z'.
  static vtkSmartPointer<vtkTransform> matFromVecR(const vtkVector3d& x,
                                                   const vtkVector3d& pos);

  Ui::RenderWindow*                   ui;

  // VTK
  QVTKWidget*                         mVtkWidget;
  vtkSmartPointer<vtkRenderer>        mRen;
  vtkSmartPointer<vtkArrowSource>     mArrowSource;
  vtkSmartPointer<vtkPolyDataMapper>  mArrowMapper;
  vtkSmartPointer<vtkTextActor>       mTxt;
};

}
}

#endif // RENDERWINDOW_H

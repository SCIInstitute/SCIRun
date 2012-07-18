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

#include "RenderWindow.h"
#include "ui_RenderWindow.h"
#include "Core/Datatypes/DenseMatrix.h"

#include <QVTKWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkArrowSource.h>
#include <vtkVector.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

using namespace SCIRun::Domain::Datatypes;

//-----------------------------------------------------------------------------
RenderWindow::RenderWindow(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::RenderWindow),
  mVtkWidget(new QVTKWidget(this, QFlag(0)))
{
  ui->setupUi(this);

  // Add VTK widget to vertical layout.
  ui->verticalLayout->addWidget(mVtkWidget);
  ui->verticalLayout->update();

  // Create renderer
  mRen = vtkSmartPointer<vtkRenderer>::New();
  mVtkWidget->GetRenderWindow()->AddRenderer(mRen);
  mVtkWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  //mRen->SetBackground(1.0,0.0,0.0);

  // Create arrow poly data
  mArrowSource = vtkSmartPointer<vtkArrowSource>::New();
  //arrowSource->SetShaftRadius(1.0);
  //arrowSource->SetTipLength(1.0);
  mArrowSource->Update();

  // Create data mapper (from visualization system to graphics system).
  mArrowMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mArrowMapper->SetInputConnection(mArrowSource->GetOutputPort());

  //setupDefaultVectorField();
  setupHelixVectorField();

  setText("This is a vector field!");
}


//-----------------------------------------------------------------------------
RenderWindow::~RenderWindow()
{
  delete mVtkWidget;
  delete ui;
}


//-----------------------------------------------------------------------------
void RenderWindow::setupDefaultVectorField()
{
  // Read matrix and build scene with appropriate actors.
  const int numVecs = 6;
  double vpd[3][numVecs] = { // Vector position data
    { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0 },
    { 0.0, 1.0, 0.0, 1.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0, 1.0, 0.0, 1.0 }
  };
  double vod[3][numVecs] = { // Vector orientation data
    { 1.0, 0.0, 0.0, 0.0, 1.0, 1.0 },
    { 0.0, 1.0, 0.0, 1.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0, 1.0, 0.0, 1.0 }
  };
    
  // Setup actors for each of the vectors...
  // There are better ways of representing vectors fields in VTK.

  // Need to construct 3x3 rotation matrices from orientation vector.
  for (int i = 0; i < numVecs; i++)
  {
    vtkSmartPointer<vtkTransform> orient = matFromVec(
        vtkVector3d(vod[0][i], vod[1][i], vod[2][i]),
        vtkVector3d(vpd[0][i], vpd[1][i], vpd[2][i]));

    vtkSmartPointer<vtkMatrix4x4> m = vtkSmartPointer<vtkMatrix4x4>::New();
    orient->GetMatrix(m);
    //m->Print(std::cout);

    // Now we have appropriate transformation data, build actor.
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mArrowMapper);
    actor->SetUserTransform(orient);
    //actor->SetPosition(vpd[0][i], vpd[1][i], vpd[2][i]);
    mRen->AddActor(actor);
  }
}

//-----------------------------------------------------------------------------
void RenderWindow::setupHelixVectorField()
{
  double t = 0.0;
  const double dt = 0.4;
  const double p2Off = 3.14159265 / 2.0f;
  vtkVector3d p1;
  vtkVector3d p2;
  vtkVector3d dp1;
  vtkVector3d dp2;
  const double heightMult = 2.0;
  const double r = 2.0; // Radius
  const int steps = 40;
  
  DenseMatrixGeneric<double> m(6, steps * 2);
  
  for (int i = 0; i < steps; i++)
  {
    p1.Set(cos(t) * r, t * heightMult, sin(t) * r);
    dp1.Set(-sin(t) * r, heightMult, cos(t) * r);
    p2.Set(cos(t + p2Off) * r, t * heightMult, sin(t + p2Off) * r);
    dp2.Set(-sin(t + p2Off) * r, heightMult, cos(t + p2Off) * r);

    int off = i * 2;

    m(0,off) = dp1.X();
    m(1,off) = dp1.Y();
    m(2,off) = dp1.Z();
    m(3,off) = p1.X();
    m(4,off) = p1.Y();
    m(5,off) = p1.Z();

    m(0,off+1) = dp2.X();
    m(1,off+1) = dp2.Y();
    m(2,off+1) = dp2.Z();
    m(3,off+1) = p2.X();
    m(4,off+1) = p2.Y();
    m(5,off+1) = p2.Z();

    t += dt;
  }

  setVectorField(m);
}

//-----------------------------------------------------------------------------
void RenderWindow::setText(const char* output)
{
  vtkSmartPointer<vtkTextActor> txt = vtkSmartPointer<vtkTextActor>::New();
  txt->SetDisplayPosition(90, 50);
  txt->SetInput(output);

  vtkTextProperty* txtProp = txt->GetTextProperty();
  txtProp->SetFontSize(18);
  txtProp->BoldOn();

  mRen->AddActor2D(txt);
}

//-----------------------------------------------------------------------------
void RenderWindow::setVectorField(const DenseMatrixGeneric<double>& m)
{
  if (m.nrows() != 6)
    throw std::runtime_error("Expecting a 6xn matrix.");

  for (int i = 0; i < m.ncols(); i++)
  {
    // Extract direction/position
    vtkVector3d dir(m(0,i), m(1,i), m(2,i));
    vtkVector3d pos(m(3,i), m(4,i), m(5,i));

    vtkSmartPointer<vtkTransform> orient = matFromVecR(dir, pos);

    //vtkSmartPointer<vtkMatrix4x4> m = vtkSmartPointer<vtkMatrix4x4>::New();
    //orient->GetMatrix(m);
    //m->Print(std::cout);

    // Now we have appropriate transformation data, build actor.
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mArrowMapper);
    actor->SetUserTransform(orient);
    //actor->SetPosition(pos.X(), pos.Y(), pos.Z());
    mRen->AddActor(actor);
  }
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkTransform> RenderWindow::matFromVec(const vtkVector3d& v,
                                                       const vtkVector3d& pos)
{
  // Find the maximal component in the vector. The construct a normal vector
  // which has a zero for the maximal component.
  float absX = fabs(v.GetX());
  float absY = fabs(v.GetY());
  float absZ = fabs(v.GetZ());

  vtkVector3d gen;
  if (absX > absY)
  {
    if (absZ > absX)
    {
      gen = vtkVector3d(1.0, 1.0, 0.0);
    }
    else
    {
      gen = vtkVector3d(0.0, 1.0, 1.0);
    }
  }
  else
  {
    if (absZ > absY)
    {
      gen = vtkVector3d(1.0, 1.0, 0.0);
    }
    else
    {
      gen = vtkVector3d(1.0, 0.0, 1.0);
    }
  }

  gen.Normalize();

  vtkVector3d at = v;
  at.Normalize();

  // Cross generated vector with 'at' vector.
  vtkVector3d right = gen.Cross(at);
  right.Normalize();
  vtkVector3d up = at.Cross(right);
  up.Normalize();

  double m[16];

  /// @todo Combine two matrices below.
  // Assuming the matrix is row major. Although, this doesn't matter for
  // the identity (I^T = I)
  m[0 ] = 1.0; m[1 ] = 0.0; m[2 ] = 0.0; m[3 ] = 0.0;
  m[4 ] = 0.0; m[5 ] = 1.0; m[6 ] = 0.0; m[7 ] = 0.0;
  m[8 ] = 0.0; m[9 ] = 0.0; m[10] = 1.0; m[11] = 0.0;
  m[12] = 0.0; m[13] = 0.0; m[14] = 0.0; m[15] = 1.0;

  // Populate rotation.  
  m[0 ] = right[0]; m[1 ] = up[0]; m[2 ] = at[0]; m[3 ] = pos.X();
  m[4 ] = right[1]; m[5 ] = up[1]; m[6 ] = at[1]; m[7 ] = pos.Y();
  m[8 ] = right[2]; m[9 ] = up[2]; m[10] = at[2]; m[11] = pos.Z();

  vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
  t->SetMatrix(m);
  //t->Inverse();

  return t;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkTransform> RenderWindow::matFromVecR(const vtkVector3d& x,
                                                        const vtkVector3d& pos)
{
  // Find the maximal component in the vector. The construct a normal vector
  // which has a zero for the maximal component.
  float absX = fabs(x.GetX());
  float absY = fabs(x.GetY());
  float absZ = fabs(x.GetZ());

  vtkVector3d gen;
  if (absX > absY)
  {
    if (absZ > absX)
    {
      gen = vtkVector3d(1.0, 1.0, 0.0);
    }
    else
    {
      gen = vtkVector3d(0.0, 1.0, 1.0);
    }
  }
  else
  {
    if (absZ > absY)
    {
      gen = vtkVector3d(1.0, 1.0, 0.0);
    }
    else
    {
      gen = vtkVector3d(1.0, 0.0, 1.0);
    }
  }

  gen.Normalize();

  vtkVector3d right = x;
  right.Normalize();

  // Cross generated vector with 'at' vector.
  vtkVector3d at = gen.Cross(right);
  at.Normalize();
  vtkVector3d up = at.Cross(right);
  up.Normalize();

  double m[16];

  /// @todo Combine two matrices below.
  // Assuming the matrix is row major. Although, this doesn't matter for
  // the identity (I^T = I)
  m[0 ] = 1.0; m[1 ] = 0.0; m[2 ] = 0.0; m[3 ] = 0.0;
  m[4 ] = 0.0; m[5 ] = 1.0; m[6 ] = 0.0; m[7 ] = 0.0;
  m[8 ] = 0.0; m[9 ] = 0.0; m[10] = 1.0; m[11] = 0.0;
  m[12] = 0.0; m[13] = 0.0; m[14] = 0.0; m[15] = 1.0;

  // Populate rotation.
  m[0 ] = right[0]; m[1 ] = up[0]; m[2 ] = at[0]; m[3 ] = pos.X();
  m[4 ] = right[1]; m[5 ] = up[1]; m[6 ] = at[1]; m[7 ] = pos.Y();
  m[8 ] = right[2]; m[9 ] = up[2]; m[10] = at[2]; m[11] = pos.Z();

  vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
  t->SetMatrix(m);
  //t->Inverse();

  return t;
}


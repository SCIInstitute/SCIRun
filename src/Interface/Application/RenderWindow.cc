#include "RenderWindow.h"
#include "ui_RenderWindow.h"

RenderWindow::RenderWindow(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::RenderWindow),
  mVtkWidget(new QVTKWidget(this, QFlag(0)))
{
  ui->setupUi(this);
  ui->verticalLayout->addWidget(mVtkWidget);
  ui->verticalLayout->update();
  mRen = vtkRenderer::New();
  mVtkWidget->GetRenderWindow()->AddRenderer(mRen);
  mRen->SetBackground(1.0,0.0,0.0);
  mRen->Render();
}

RenderWindow::~RenderWindow()
{
  mRen->Delete();
  delete mVtkWidget;
  delete ui;
}

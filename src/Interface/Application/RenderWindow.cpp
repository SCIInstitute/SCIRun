#include "RenderWindow.h"
#include "ui_RenderWindow.h"

RenderWindow::RenderWindow(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::RenderWindow)
{
  ui->setupUi(this);

}

RenderWindow::~RenderWindow()
{
  delete ui;
}

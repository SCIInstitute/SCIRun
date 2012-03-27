#include <QtGui>
#include "Module.h"

using namespace SCIRun::Gui;

Module::Module(const QString& name, QWidget* parent /* = 0 */)
  : QFrame(parent)
{
  setupUi(this);
  label->setText(name);
  
}
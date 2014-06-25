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

#include <Interface/Modules/Math/SolveLinearSystemDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Logging/Log.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <boost/bimap.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

namespace SCIRun {
  namespace Gui {
    class SolveLinearSystemDialogImpl
    {
    public:
      SolveLinearSystemDialogImpl()
      {
        typedef boost::bimap<std::string, std::string>::value_type strPair;
        solverNameLookup_.insert(strPair("Conjugate Gradient (SCI)", "cg"));
        solverNameLookup_.insert(strPair("BiConjugate Gradient (SCI)", "bicg"));
        solverNameLookup_.insert(strPair("Jacobi (SCI)", "jacobi"));
        solverNameLookup_.insert(strPair("MINRES (SCI)", "minres"));
      }
      boost::bimap<std::string, std::string> solverNameLookup_;
    };
  }}

SolveLinearSystemDialog::SolveLinearSystemDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent), impl_(new SolveLinearSystemDialogImpl)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addSpinBoxManager(maxIterationsSpinBox_, Variables::MaxIterations);
  addDoubleSpinBoxManager(targetErrorSpinBox_, Variables::TargetError);
  //TODO: clean these up...still getting circles of push/pull
  //TODO: need this connection ???
  connect(methodComboBox_, SIGNAL(activated(const QString&)), this, SLOT(pushParametersToState()));
  //connect(preconditionerComboBox_, SIGNAL(activated(const QString&)), this, SLOT(pushParametersToState()));
  addComboBoxManager(preconditionerComboBox_, Variables::Preconditioner);
}

void SolveLinearSystemDialog::pushParametersToState()
{
  if (!pulling_)
  {
    //TODO: need pattern for this, to avoid silly recursion of push/pull.

    {
      auto method = methodComboBox_->currentText().toStdString();
      Core::Logging::Log::get() << Core::Logging::DEBUG_LOG << "GUI: METHOD SELECTED: " << method;
      Core::Logging::Log::get().flush();

      std::string methodOption = impl_->solverNameLookup_.left.at(method);

      if (methodOption != state_->getValue(Variables::Method).getString())
      {
        state_->setValue(Variables::Method, methodOption);
      }
    }

//     {
//       QString precond = preconditionerComboBox_->currentText();
//       std::string precondOption;
//       if (precond == "Jacobi")
//         precondOption = "jacobi";
//       else 
//         precondOption = "None";
// 
//       if (precondOption != state_->getValue(Variables::Preconditioner).getString())
//       {
//         state_->setValue(Variables::Preconditioner, precondOption);
//       }
//     }
  }
}

void SolveLinearSystemDialog::pull()
{
  //TODO convert to new widget managers
  Pulling p(this);
  
  auto method = state_->getValue(Variables::Method).getString();
  
  auto it = impl_->solverNameLookup_.right.find(method);
  if (it != impl_->solverNameLookup_.right.end())
    methodComboBox_->setCurrentIndex(methodComboBox_->findText(QString::fromStdString(it->get_left())));

  //auto precond = state_->getValue(Variables::Preconditioner).getString();
  //preconditionerComboBox_->setCurrentIndex((precond == "jacobi") ? 0 : 1);
}


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

#include <Interface/Modules/Fields/MapFieldDataFromNodeToElemDialog.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/MapFieldDataFromNodeToElem.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Logging/Log.h>
#include <boost/bimap.hpp>
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;


namespace SCIRun {
  namespace Gui {
    class MapFieldDataFromNodeToElemDialogImpl
    {
    public:
      MapFieldDataFromNodeToElemDialogImpl()
      {
        typedef boost::bimap<std::string, std::string>::value_type strPair;
        MethodNameLookup_.insert(strPair("Interpolation", "interpolation"));
        MethodNameLookup_.insert(strPair("Average", "average"));
        MethodNameLookup_.insert(strPair("Min", "min"));
        MethodNameLookup_.insert(strPair("Max", "max"));
	MethodNameLookup_.insert(strPair("Sum", "sum"));
	MethodNameLookup_.insert(strPair("Median", "median"));
	MethodNameLookup_.insert(strPair("None", "none"));
      }
      boost::bimap<std::string, std::string> MethodNameLookup_;
    };
  }}

MapFieldDataFromNodeToElemDialog::MapFieldDataFromNodeToElemDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent), impl_(new MapFieldDataFromNodeToElemDialogImpl)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  connect(methodComboBox_, SIGNAL(activated(const QString&)), this, SLOT(push()));
}

void MapFieldDataFromNodeToElemDialog::push()
{
  if (!pulling_)
  {
    auto method = methodComboBox_->currentText().toStdString();
    Core::Logging::Log::get() << Core::Logging::DEBUG_LOG << "GUI: METHOD SELECTED: " << method;
    Core::Logging::Log::get().flush();
    std::string methodOption = impl_->MethodNameLookup_.left.at(method);
    if (methodOption != state_->getValue(MapFieldDataFromNodeToElemAlgo::Method).getString())
      {
        state_->setValue(MapFieldDataFromNodeToElemAlgo::Method, methodOption);
      }
  }
}

//BIG DAN TODO: extract class for Widget/StateVar interaction. Starting to look like Seg3D code...

void MapFieldDataFromNodeToElemDialog::pull()
{
  Pulling p(this);
 
  auto method = state_->getValue(MapFieldDataFromNodeToElemAlgo::Method).getString();
  
  auto it = impl_->MethodNameLookup_.right.find(method);
  if (it != impl_->MethodNameLookup_.right.end())
    methodComboBox_->setCurrentIndex(methodComboBox_->findText(QString::fromStdString(it->get_left())));  
 
}

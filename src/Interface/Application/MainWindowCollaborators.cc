/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <iostream>
#include <QtGui>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Logging;

void TextEditAppender::log(const QString& message) const
{
  text_->append(message);
}

void TextEditAppender::error(const std::string& msg) const
{
  log("Error: " + QString::fromStdString(msg));
  Log::get() << ERROR_LOG << msg << std::endl;
}

void TextEditAppender::warning(const std::string& msg) const
{
  log("Warning: " + QString::fromStdString(msg));
  Log::get() << WARN << msg << std::endl;
}

void TextEditAppender::remark(const std::string& msg) const
{
  log("Remark: " + QString::fromStdString(msg));
  Log::get() << NOTICE << msg << std::endl;
}

void TextEditAppender::status(const std::string& msg) const
{
  log(QString::fromStdString(msg));
  Log::get() << DEBUG_LOG << msg << std::endl;
}

QString TreeViewModuleGetter::text() const
{
  return tree_.currentItem()->text(0);
}

bool TreeViewModuleGetter::isModule() const
{
  auto current = tree_.currentItem();
  return current->childCount() == 0 && current->parent();
}

NotePosition ComboBoxDefaultNotePositionGetter::position() const
{
  return NotePosition(combo_.currentIndex() + 1);
}

CORE_SINGLETON_IMPLEMENTATION( WidgetDisablingService )

namespace
{
  class SetDisableFlag : public boost::static_visitor<>
  {
  public:
    explicit SetDisableFlag(bool flag) : flag_(flag) {}
    template <typename T>
    void operator()( T* widget ) const
    {
      //TODO: investigate this Mac Qt bug in more detail. A better workaround probably exists. (Or just wait until Qt 5)
//#ifdef WIN32
      if (widget)
        widget->setDisabled(flag_);
//#endif
    }
    bool flag_;
  };

  void setWidgetsDisableFlag(std::vector<InputWidget>& widgets, bool flag)
  {
    std::for_each(widgets.begin(), widgets.end(), [=](InputWidget& v) { boost::apply_visitor(SetDisableFlag(flag), v); });
  }
}

void WidgetDisablingService::addNetworkEditor(NetworkEditor* ne)
{
  ne_ = ne;
}

void WidgetDisablingService::addWidget(const InputWidget& w)
{
  inputWidgets_.push_back(w);
}

void WidgetDisablingService::removeWidget(const InputWidget& w)
{
  inputWidgets_.erase(std::remove(inputWidgets_.begin(), inputWidgets_.end(), w));
}

void WidgetDisablingService::disableInputWidgets()
{
  ne_->disableInputWidgets();
  setWidgetsDisableFlag(inputWidgets_, true);
  //qDebug() << "disabling widgets; service on?" << serviceEnabled_;
}

void WidgetDisablingService::enableInputWidgets()
{
  ne_->enableInputWidgets();
  setWidgetsDisableFlag(inputWidgets_, false);
  //qDebug() << "enabling widgets; service on?" << serviceEnabled_;
}

void WidgetDisablingService::temporarilyDisableService()
{
  //qDebug() << "temp disable service";
  serviceEnabled_ = false;
}

void WidgetDisablingService::temporarilyEnableService()
{
  //qDebug() << "temp enable service";
  serviceEnabled_ = true;
}

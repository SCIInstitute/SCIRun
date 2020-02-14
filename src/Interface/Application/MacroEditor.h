/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

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


#ifndef INTERFACE_APPLICATION_MACROEDITOR_H
#define INTERFACE_APPLICATION_MACROEDITOR_H

#include "ui_MacroEditor.h"
#include <QMap>

#ifndef Q_MOC_RUN
#include <Dataflow/Network/NetworkFwd.h>
#endif

namespace SCIRun {
namespace Gui {

  class NetworkEditor;
  class CodeEditor;

  using MacroNameValueList = QList<QStringList>;

  enum MacroListItem
  {
    Name,
    Script,
    ButtonNumber
  };

class MacroEditor : public QDockWidget, public Ui::MacroEditor
{
	Q_OBJECT

public:
  explicit MacroEditor(QWidget* parent = nullptr);
  const MacroNameValueList& scripts() const;
  void setScripts(const MacroNameValueList& scripts);

  QString macroForButton(int i) const;

  static const char* Index;
  enum
  {
    MIN_MACRO_INDEX = 1,
    MAX_MACRO_INDEX = 5
  };

Q_SIGNALS:
  void macroButtonChanged(int index, const QString& name);

private Q_SLOTS:
  void updateScriptEditor();
  void updateScripts();
  void addMacro();
  void removeMacro();
  void renameMacro();
  void assignToButton();
  void runSelectedMacro();

private:
  void highlightButton(QPushButton* button) const;
  void dehighlightButton(QPushButton* button) const;
  CodeEditor* scriptPlainTextEdit_;
  MacroNameValueList macros_;
  std::vector<QPushButton*> buttons_;
};

}
}

#endif

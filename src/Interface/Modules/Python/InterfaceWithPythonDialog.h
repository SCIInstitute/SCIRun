/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef INTERFACE_MODULES_PYTHON_INTERFACEWITHPYTHON_H
#define INTERFACE_MODULES_PYTHON_INTERFACEWITHPYTHON_H

#include "Interface/Modules/Python/ui_InterfaceWithPython.h"
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Interface/Modules/Python/share.h>

namespace SCIRun {
namespace Gui {

class SCISHARE InterfaceWithPythonDialog : public ModuleDialogGeneric,
  public Ui::InterfaceWithPython
{
	Q_OBJECT

public:
  InterfaceWithPythonDialog(const std::string& name, SCIRun::Dataflow::Networks::ModuleStateHandle state, QWidget* parent = nullptr);
public Q_SLOTS:
  virtual void updateFromPortChange(int numPorts, const std::string& portName, DynamicPortChange type) override;
private Q_SLOTS:
  void resetObjects();
  void loadAPIDocumentation();
private:
  void handleInputTableWidgetRowChange(const std::string& portId, const std::string& type, DynamicPortChange portChangeType);
  void setupOutputTableCells();
  class CodeEditor* pythonCodePlainTextEdit_;
};

class CodeEditor : public QPlainTextEdit
{
  Q_OBJECT

public:
  explicit CodeEditor(QWidget *parent = nullptr);

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

public Q_SLOTS:
  void insertSpecialCodeBlock();
protected:
  void resizeEvent(QResizeEvent *event) override;

private Q_SLOTS:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &, int);
  void matchParentheses();

private:
  QWidget* lineNumberArea_;
  class Highlighter* highlighter_;
  void createParenthesisSelection(int pos);
  bool matchLeftParenthesis(QTextBlock currentBlock, int index, int numRightParentheses);
  bool matchRightParenthesis(QTextBlock currentBlock, int index, int numLeftParentheses);
};

class LineNumberArea : public QWidget
{
public:
  explicit LineNumberArea(CodeEditor *editor) : QWidget(editor) 
  {
    codeEditor = editor;
  }

  QSize sizeHint() const override 
  {
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
  }

protected:
  void paintEvent(QPaintEvent *event) override 
  {
    codeEditor->lineNumberAreaPaintEvent(event);
  }

private:
  CodeEditor *codeEditor;
};

class Highlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  Highlighter(QTextDocument* parent = nullptr);

protected:
  void highlightBlock(const QString &text) override;

private:
  struct HighlightingRule
  {
    QRegExp pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> highlightingRules;

  QRegExp commentStartExpression;
  QRegExp commentEndExpression;

  QTextCharFormat keywordFormat;
  QTextCharFormat classFormat;
  QTextCharFormat singleLineCommentFormat;
  QTextCharFormat multiLineCommentFormat;
  QTextCharFormat quotationFormat;
  QTextCharFormat functionFormat;

  void highlightBlockParens(const QString& text);
};

struct ParenthesisInfo
{
  char character;
  int position;
};

class TextBlockData : public QTextBlockUserData
{
public:
  TextBlockData();
  QVector<ParenthesisInfo *> parentheses();
  void insert(ParenthesisInfo *info);
private:
  QVector<ParenthesisInfo *> m_parentheses;
};

}
}

#endif

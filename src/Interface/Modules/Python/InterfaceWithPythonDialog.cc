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

#include <Interface/Modules/Python/InterfaceWithPythonDialog.h>
#include <Modules/Python/InterfaceWithPython.h>
#include <Modules/Python/PythonObjectForwarder.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Python;

InterfaceWithPythonDialog::InterfaceWithPythonDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  {
    pythonCodePlainTextEdit_ = new CodeEditor(this);
    tabWidget->widget(0)->layout()->addWidget(pythonCodePlainTextEdit_);
  }

  addPlainTextEditManager(pythonCodePlainTextEdit_, Parameters::PythonCode);

  addSpinBoxManager(retryAttemptsSpinBox_, Parameters::NumberOfRetries);
  addSpinBoxManager(pollingIntervalSpinBox_, Parameters::PollingIntervalMilliseconds);

  connect(clearObjectPushButton_, SIGNAL(clicked()), this, SLOT(resetObjects()));

  WidgetStyleMixin::tabStyle(tabWidget);
  WidgetStyleMixin::tableHeaderStyle(inputVariableNamesTableWidget_);
  WidgetStyleMixin::tableHeaderStyle(outputVariableNamesTableWidget_);

  setupOutputTableCells();

  connect(pythonDocPushButton_, SIGNAL(clicked()), this, SLOT(loadAPIDocumentation()));
}

void InterfaceWithPythonDialog::resetObjects()
{
  for (const auto& objName : SCIRun::Modules::Python::InterfaceWithPython::outputNameParameters())
    state_->setTransientValue(state_->getValue(objName).toString(), boost::any());
}

void InterfaceWithPythonDialog::setupOutputTableCells()
{
  auto outputNames = SCIRun::Modules::Python::InterfaceWithPython::outputNameParameters();
  for (int i = 0; i < outputVariableNamesTableWidget_->rowCount(); ++i)
  {
    for (int j = 0; j < outputVariableNamesTableWidget_->columnCount(); ++j)
    {
      if (j == outputVariableNamesTableWidget_->columnCount() - 1)
      {
        auto lineEdit = new QLineEdit;
        addLineEditManager(lineEdit, outputNames[i]);
        outputVariableNamesTableWidget_->setCellWidget(i, outputVariableNamesTableWidget_->columnCount() - 1, lineEdit);
      }
      else
      {
        auto item = outputVariableNamesTableWidget_->item(i, j);
        if (item)
        {
          item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
      }
    }
  }
}

void InterfaceWithPythonDialog::updateFromPortChange(int numPorts, const std::string& portId, DynamicPortChange type)
{
  if (type == DynamicPortChange::INITIAL_PORT_CONSTRUCTION)
    return;

  if (type == DynamicPortChange::USER_REMOVED_PORT)
  {
    QMessageBox::warning(this, "Warning: possible Python code update required", windowTitle() +
      ": The connection to port " + QString::fromStdString(portId) + " was deleted. The variable name \"" +
      QString::fromStdString(state_->getValue(SCIRun::Core::Algorithms::Name(portId)).toString()) + "\" is no longer valid."
      + " Please update your Python code or input variable table to reflect this.");
  }

  inputVariableNamesTableWidget_->blockSignals(true);

  handleInputTableWidgetRowChange(portId, "Matrix", type);
  handleInputTableWidgetRowChange(portId, "Field", type);
  handleInputTableWidgetRowChange(portId, "String", type);

  inputVariableNamesTableWidget_->resizeColumnsToContents();
  inputVariableNamesTableWidget_->blockSignals(false);
}

void InterfaceWithPythonDialog::handleInputTableWidgetRowChange(const std::string& portId, const std::string& type, DynamicPortChange portChangeType)
{
  const int lineEditColumn = 2;
  syncTableRowsWithDynamicPort(portId, type, inputVariableNamesTableWidget_, lineEditColumn, portChangeType,
    { { 1, [&](){ return new QTableWidgetItem(QString::fromStdString(type)); } } });
}

void InterfaceWithPythonDialog::loadAPIDocumentation()
{
  openPythonAPIDoc();
}

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea_ = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    highlighter_ = new Highlighter(document());
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea_->scroll(0, dy);
    else
        lineNumberArea_->update(0, rect.y(), lineNumberArea_->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea_->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::darkGray);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea_);
    painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
   int blockNumber = block.blockNumber();
   int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
   int bottom = top + (int) blockBoundingRect(block).height();
   while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
        QString number = QString::number(blockNumber + 1);
        painter.setPen(Qt::black);
        painter.drawText(0, top, lineNumberArea_->width(), fontMetrics().height(),
                         Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + (int) blockBoundingRect(block).height();
    ++blockNumber;
}
}

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::green);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bfor\\b" << "\\bin\\b"
                    << "\\bTrue\\b" << "\\bFalse\\b";
    for (const auto& pattern : keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
    classFormat.setFontWeight(QFont::Bold);
classFormat.setForeground(Qt::darkMagenta);
rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
rule.format = classFormat;
highlightingRules.append(rule);

quotationFormat.setForeground(Qt::darkGreen);
rule.pattern = QRegExp("\".*\"");
rule.format = quotationFormat;
highlightingRules.append(rule);

functionFormat.setFontItalic(true);
functionFormat.setForeground(Qt::cyan);
rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
rule.format = functionFormat;
highlightingRules.append(rule);

singleLineCommentFormat.setForeground(Qt::yellow);
rule.pattern = QRegExp("#[^\n]*");
rule.format = singleLineCommentFormat;
highlightingRules.append(rule);

multiLineCommentFormat.setForeground(Qt::yellow);

commentStartExpression = QRegExp("/\\*");
commentEndExpression = QRegExp("\\*/");
}

void Highlighter::highlightBlock(const QString &text)
{
    for (const auto& rule : highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
        setCurrentBlockState(0);
        int startIndex = 0;
if (previousBlockState() != 1)
    startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {
    int endIndex = commentEndExpression.indexIn(text, startIndex);
    int commentLength;
    if (endIndex == -1) {
        setCurrentBlockState(1);
        commentLength = text.length() - startIndex;
    } else {
        commentLength = endIndex - startIndex
                        + commentEndExpression.matchedLength();
    }
    setFormat(startIndex, commentLength, multiLineCommentFormat);
    startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
}
}

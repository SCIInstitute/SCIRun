/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <QtGui>

#include <Interface/PythonTestGui/Node.h>
#include <Interface/PythonTestGui/PropertiesDialog.h>

PropertiesDialog::PropertiesDialog(Node *node, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    this->node = node;
    xSpinBox->setValue(int(node->x()));
    ySpinBox->setValue(int(node->y()));
    textLineEdit->setText(node->text());
    textColor = node->textColor();
    outlineColor = node->outlineColor();
    backgroundColor = node->backgroundColor();

    updateColorLabel(outlineColorLabel, outlineColor);
    updateColorLabel(backgroundColorLabel, backgroundColor);
    updateColorLabel(textColorLabel, textColor);
}

void PropertiesDialog::on_buttonBox_accepted()
{
    node->setPos(xSpinBox->value(), ySpinBox->value());
    node->setText(textLineEdit->text());
    node->setOutlineColor(outlineColor);
    node->setBackgroundColor(backgroundColor);
    node->setTextColor(textColor);
    node->update();
    QDialog::accept();
}

void PropertiesDialog::on_textColorButton_clicked()
{
    chooseColor(textColorLabel, &textColor);
}

void PropertiesDialog::on_outlineColorButton_clicked()
{
    chooseColor(outlineColorLabel, &outlineColor);
}

void PropertiesDialog::on_backgroundColorButton_clicked()
{
    chooseColor(backgroundColorLabel, &backgroundColor);
}

void PropertiesDialog::updateColorLabel(QLabel *label,
                                        const QColor &color)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    label->setPixmap(pixmap);
}

void PropertiesDialog::chooseColor(QLabel *label, QColor *color)
{
    QColor newColor = QColorDialog::getColor(*color, this);
    if (newColor.isValid()) {
        *color = newColor;
        updateColorLabel(label, *color);
    }
}

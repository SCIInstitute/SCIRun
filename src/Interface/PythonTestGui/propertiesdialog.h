/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include "ui_propertiesdialog.h"

class Node;

class PropertiesDialog : public QDialog, private Ui::PropertiesDialog
{
    Q_OBJECT

public:
    PropertiesDialog(Node *node, QWidget *parent = 0);

private Q_SLOTS:
    void on_buttonBox_accepted();
    void on_textColorButton_clicked();
    void on_outlineColorButton_clicked();
    void on_backgroundColorButton_clicked();

private:
    void updateColorLabel(QLabel *label, const QColor &color);
    void chooseColor(QLabel *label, QColor *color);

    Node *node;
    QColor textColor;
    QColor outlineColor;
    QColor backgroundColor;
};

#endif

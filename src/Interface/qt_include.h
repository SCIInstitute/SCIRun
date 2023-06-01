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


#include <QtWidgets>

#ifdef OLDER_QT_SUPPORT_NEEDED // remove when Travis linux build has newer Qt 5 version
#define WIDTH_FUNC width
#define COMBO_BOX_ACTIVATED_STRING qOverload<const QString&>(&QComboBox::activated)
#define BUTTON_GROUP_SIGNAL qOverload<int>(&QButtonGroup::buttonPressed)
#else
#define WIDTH_FUNC horizontalAdvance
#define COMBO_BOX_ACTIVATED_STRING &QComboBox::textActivated
#define BUTTON_GROUP_SIGNAL &QButtonGroup::idClicked
#endif

#ifdef SCIRUN_QT6_ENABLED
#define Q_ENTER_EVENT_CLASS QEnterEvent
#else
#define Q_ENTER_EVENT_CLASS QEvent
#endif

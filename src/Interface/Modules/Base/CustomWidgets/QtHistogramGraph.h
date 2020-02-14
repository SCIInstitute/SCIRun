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


#ifndef QTUTILS_WIDGETS_QTHISTOGRAMGRAPH_H
#define QTUTILS_WIDGETS_QTHISTOGRAMGRAPH_H

#ifndef Q_MOC_RUN

#include <vector>
#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <Core/Math/Histogram.h>
#include <Interface/Modules/Base/share.h>

#endif

namespace SCIRun
{
  namespace QtUtils
  {

    class SCISHARE QtHistogramGraph : public QWidget
    {
      Q_OBJECT

      Q_SIGNALS :
      void lower_position(int);
      void upper_position(int);

    public:
      explicit QtHistogramGraph(QWidget *parent = nullptr);

    public:
      /// SET_HISTOGRAM:
      /// Set the histogram of the graph
      void set_histogram(const Core::Math::Histogram& histogram);

      /// RESET_HISTOGRAM:
      /// Invalidate the current histogram
      void reset_histogram();

      bool get_logarithmic() const{ return this->logarithmic_; }

      public Q_SLOTS:
      void set_logarithmic(bool logarithmic);

    protected:
      /// PAINTEVENT:
      /// Overloaded call that redraws the histogram plot
      virtual void paintEvent(QPaintEvent *event);

    public:
      virtual void mousePressEvent(QMouseEvent* e);

      virtual void mouseMoveEvent(QMouseEvent* e);

    private:
      Core::Math::Histogram histogram_;
      bool logarithmic_;
      bool left_click_;

    };

  }
}

#endif

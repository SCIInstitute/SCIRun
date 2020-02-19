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


#ifndef QTUTILS_WIDGETS_QTHISTOGRAMWIDGET_H
#define QTUTILS_WIDGETS_QTHISTOGRAMWIDGET_H

#ifndef Q_MOC_RUN
#include <vector>
#include <boost/shared_ptr.hpp>
#include <QWidget>
#include <Core/Math/Histogram.h>
#include <Interface/Modules/Base/share.h>

// QtUtils includes
//#include <QtUtils/Widgets/QtSliderDoubleCombo.h>

#endif

namespace SCIRun
{
  namespace QtUtils
  {
    class QtHistogramWidgetPrivate;
    typedef boost::shared_ptr< QtHistogramWidgetPrivate > QtHistogramWidgetPrivateHandle;

    class SCISHARE QtHistogramWidget : public QWidget
    {
      Q_OBJECT

      Q_SIGNALS :
      void lower_changed_signal(double);
      void upper_changed_signal(double);

    public:
      QtHistogramWidget(QWidget *parent,
        bool show_threshold_brackets = false
        //QtSliderDoubleCombo* upper_threshold = 0,
        //QtSliderDoubleCombo* lower_threshold = 0
        );
      virtual ~QtHistogramWidget();

    public Q_SLOTS:
      void set_histogram(const Core::Math::Histogram& histogram);

      void set_min(double);
      void set_max(double);

      double get_histogram_min();
      double get_histogram_max();

      void hide_threshold_bars();
      void show_threshold_bars();

      /// RESET_HISTOGRAM:
      /// Invalidate the current histogram
      void reset_histogram();

    public:
      virtual void mousePressEvent(QMouseEvent* e);

      virtual void mouseMoveEvent(QMouseEvent* e);

      virtual void resizeEvent(QResizeEvent* e);

      //void set_thresholds(QtSliderDoubleCombo* upper_threshold, QtSliderDoubleCombo* lower_threshold);


    private Q_SLOTS:
      void handle_right_button_click(int);
      void handle_left_button_click(int);
      void set_histogram_view(int);
      void set_bar_visibility();

    private:
      QtHistogramWidgetPrivateHandle private_;

    };

  }
}

#endif

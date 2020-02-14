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


#include "ui_QtHistogramWidget.h"

#include <cmath>
#include <Interface/Modules/Base/CustomWidgets/QtHistogramWidget.h>
#include <Interface/Modules/Base/CustomWidgets/QtHistogramGraph.h>

using namespace SCIRun;
using namespace Core::Math;
using namespace QtUtils;

namespace SCIRun
{
  namespace QtUtils
  {
    class QtHistogramWidgetPrivate
    {
    public:
      Ui::HistogramWidget ui_;
      QtHistogramGraph* histogram_graph_;
      int bins_;

      double min_;
      double max_;

      double min_threshold_value_;
      double max_threshold_value_;

      bool threshold_bars_enabled_;

      //QtUtils::QtSliderDoubleCombo *upper_threshold_;
      //QtUtils::QtSliderDoubleCombo *lower_threshold_;

      QWidget* min_bar_;
      QWidget* max_bar_;
    };
  }
}


QtHistogramWidget::QtHistogramWidget( QWidget *parent,
  bool show_threshold_brackets
  //QtSliderDoubleCombo* upper_threshold,
  //QtSliderDoubleCombo* lower_threshold,
  ) :
  QWidget( parent ),
    private_( new QtHistogramWidgetPrivate )
{
  this->private_->ui_.setupUi( this );

  //this->private_->lower_threshold_ = lower_threshold;
  //this->private_->upper_threshold_ = upper_threshold;
  this->private_->threshold_bars_enabled_ = show_threshold_brackets;

  this->private_->min_threshold_value_ = 0;
  this->private_->max_threshold_value_ = 0;

  this->private_->histogram_graph_ = new QtHistogramGraph( this );
  this->private_->histogram_graph_->setObjectName( QString::fromUtf8( "histogram_" ) );
  this->private_->ui_.histogramLayout->addWidget( this->private_->histogram_graph_ );

  connect( this->private_->histogram_graph_, SIGNAL( lower_position( int ) ), this,
    SLOT( handle_right_button_click( int ) ) );

  connect( this->private_->histogram_graph_, SIGNAL( upper_position( int ) ), this,
    SLOT( handle_left_button_click( int ) ) );

  connect( this->private_->ui_.log_histogram_combo_, SIGNAL( currentIndexChanged ( int ) ),
    this, SLOT( set_histogram_view( int ) ) );
  //
  //if( this->private_->upper_threshold_ != 0 )
  //{
  //  connect( this->private_->upper_threshold_, SIGNAL( valueAdjusted( double ) ),
  //    this, SLOT( set_max( double ) ) );
  //
  //}
  //
  //if( this->private_->lower_threshold_ != 0 )
  //{
  //  connect( this->private_->lower_threshold_, SIGNAL( valueAdjusted( double ) ),
  //    this, SLOT( set_min( double ) ) );
  //}


  this->private_->min_bar_ = new QWidget( this );
  this->private_->max_bar_ = new QWidget( this );
  this->set_bar_visibility();

  QFont font = this->private_->ui_.min->font();
#ifdef __APPLE__
  font.setPointSize( 10 );
#else
  font.setPointSize( 8 );
#endif
  this->private_->ui_.min->setFont( font );
  this->private_->ui_.max->setFont( font );

  this->private_->ui_.label_3->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  // We will set the histogram to be logarithmic by default
  this->private_->ui_.log_histogram_combo_->setCurrentIndex( 1 );
}

QtHistogramWidget::~QtHistogramWidget()
{
}

void QtHistogramWidget::set_histogram( const Histogram& histogram )
{
  this->reset_histogram();
  this->private_->min_ = histogram.get_min();
  this->private_->max_ = histogram.get_max();

  this->private_->histogram_graph_->set_histogram( histogram );
  this->private_->bins_ =  static_cast< int >( histogram.get_size() );
  this->private_->ui_.min->setText( QString::number( histogram.get_min() ) );
  this->private_->ui_.max->setText( QString::number( histogram.get_max() ) );

  if( this->private_->min_bar_ ) this->private_->min_bar_->deleteLater();
  if( this->private_->max_bar_ ) this->private_->max_bar_->deleteLater();

  this->private_->histogram_graph_->repaint();

  this->private_->min_bar_ = new QWidget( this );
  this->private_->max_bar_ = new QWidget( this );
  this->set_bar_visibility();

 /* if( ( !this->private_->upper_threshold_ ) || ( this->private_->upper_threshold_->get_value() > histogram.get_max() ) )
  {
    this->set_max( histogram.get_max() );
  }
  else if( this->private_->upper_threshold_->get_value() < histogram.get_min() )
  {
    this->set_max( histogram.get_min() );
  }
  else
  {
    this->set_max( this->private_->upper_threshold_->get_value() );
  }

  if( ( !this->private_->lower_threshold_ ) || ( this->private_->lower_threshold_->get_value() < histogram.get_min() ) )
  {
    this->set_min( histogram.get_min() );
  }
  else if( this->private_->lower_threshold_->get_value() > histogram.get_max() )
  {
    this->set_min( histogram.get_max() );
  }
  else
  {
    this->set_min( this->private_->lower_threshold_->get_value() );
  }*/
}

void QtHistogramWidget::reset_histogram( )
{
  this->private_->histogram_graph_->reset_histogram();
  this->private_->ui_.min->setText( "-" );
  this->private_->ui_.max->setText( "-" );
  this->private_->histogram_graph_->repaint();
}

//#define INTERFACE_ACCENT_COLOR "128,128,128";

void QtHistogramWidget::set_min( double min )
{
  this->private_->min_threshold_value_ = min;

  double temp_max;

  if( this->private_->min_ < 0 )
  {
    min = min + std::fabs( this->private_->min_ );
    temp_max = this->private_->max_ + std::fabs(this->private_->min_);
  }
  else
  {
    min = min - this->private_->min_;
    temp_max = this->private_->max_ - this->private_->min_;
  }

  double percentage = min / temp_max;

  double min_location = ( ( this->private_->histogram_graph_->width() - 4 ) * percentage ) + 4;
  this->private_->min_bar_->setGeometry( min_location, 4 , 4, 129 );
  //this->private_->min_bar_->setStyleSheet( QString::fromUtf8( "border-left: 2px solid rgb( " INTERFACE_ACCENT_COLOR " );"
  //  "border-bottom: 2px solid rgb( " INTERFACE_ACCENT_COLOR " );"
  //  "border-top: 2px solid rgb( " INTERFACE_ACCENT_COLOR " );" ) );
}

void QtHistogramWidget::set_max( double max )
{
  this->private_->max_threshold_value_ = max;

  double temp_max;

  if( this->private_->min_ < 0 )
  {
    max = max + std::fabs(this->private_->min_);
    temp_max = this->private_->max_ + std::fabs(this->private_->min_);
  }
  else
  {
    max = max - this->private_->min_;
    temp_max = this->private_->max_ - this->private_->min_;
  }

  double percentage = max / temp_max;
  double max_location = ( ( this->private_->histogram_graph_->width() - 4 ) * percentage ) + 4;
  this->private_->max_bar_->setGeometry( max_location, 4, 4, 129 );
  //this->private_->max_bar_->setStyleSheet( QString::fromUtf8( "border-right: 2px solid rgb( " INTERFACE_ACCENT_COLOR " );"
  //  "border-bottom: 2px solid rgb( " INTERFACE_ACCENT_COLOR " );"
  //  "border-top: 2px solid rgb( " INTERFACE_ACCENT_COLOR " );" ) );
}

void QtHistogramWidget::handle_left_button_click( int lower_location )
{
  return;
/*
  if( this->private_->lower_threshold_ == 0 )
    return;

  double percent_of_width = lower_location /
    double ( this->private_->histogram_graph_->width( ) );
  double current_value = ( this->private_->max_ -
    this->private_->min_ ) * percent_of_width;
  if( this->private_->min_ < 0 ) current_value = current_value + this->private_->min_;
  this->private_->lower_threshold_->setCurrentValue( current_value );*/
}

void QtHistogramWidget::handle_right_button_click( int upper_location )
{
  return;

  //if( this->private_->upper_threshold_ == 0 ) return;

  //double percent_of_width = upper_location /
  //  double ( this->private_->histogram_graph_->width() );
  //double current_value = ( this->private_->max_ -
  //  this->private_->min_ ) * percent_of_width;
  //if( this->private_->min_ < 0 ) current_value = current_value + this->private_->min_;
  //this->private_->upper_threshold_->setCurrentValue( current_value );
}

double QtHistogramWidget::get_histogram_min()
{
  return this->private_->ui_.min->text().toDouble();
}

double QtHistogramWidget::get_histogram_max()
{
  return this->private_->ui_.max->text().toDouble();
}

void QtHistogramWidget::set_histogram_view( int mode )
{
  this->private_->histogram_graph_->set_logarithmic( mode == 1 );
}

void QtHistogramWidget::mousePressEvent( QMouseEvent* e )
{
  this->private_->histogram_graph_->mousePressEvent( e );
}

void QtHistogramWidget::mouseMoveEvent( QMouseEvent* e )
{
  this->private_->histogram_graph_->mouseMoveEvent( e );
}
//
//void QtHistogramWidget::set_thresholds( QtSliderDoubleCombo* upper_threshold, QtSliderDoubleCombo* lower_threshold )
//{
//  this->private_->upper_threshold_ = upper_threshold;
//  this->private_->lower_threshold_ = lower_threshold;
//  this->private_->threshold_bars_enabled_ = true;
//  this->set_bar_visibility();
//
//  connect( this->private_->upper_threshold_, SIGNAL( valueAdjusted( double ) ),
//    this, SLOT( set_max( double ) ) );
//
//  connect( this->private_->lower_threshold_, SIGNAL( valueAdjusted( double ) ),
//    this, SLOT( set_min( double ) ) );
//}

void QtHistogramWidget::resizeEvent( QResizeEvent* event )
{
  QWidget::resizeEvent( event );
  if( this->width() < 100 ) return;
  this->set_min( this->private_->min_threshold_value_ );
  this->set_max( this->private_->max_threshold_value_ );
}

void QtHistogramWidget::set_bar_visibility()
{
  if( this->private_->threshold_bars_enabled_ )
  {
    this->private_->min_bar_->show();
    this->private_->max_bar_->show();
  }
  else
  {
    this->private_->min_bar_->hide();
    this->private_->max_bar_->hide();
  }
}

void QtHistogramWidget::hide_threshold_bars()
{
  if( this->private_->min_bar_ != 0 && this->private_->min_bar_ != 0 )
  {
    this->private_->min_bar_->hide();
    this->private_->max_bar_->hide();
  }
}

void QtHistogramWidget::show_threshold_bars()
{
  if( this->private_->min_bar_ != 0 && this->private_->min_bar_ != 0 )
  {
    this->private_->min_bar_->show();
    this->private_->max_bar_->show();
  }
}

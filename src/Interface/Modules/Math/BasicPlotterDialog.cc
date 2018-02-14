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

#include <Interface/Modules/Math/BasicPlotterDialog.h>
//#include <Interface/Modules/Base/CustomWidgets/QtHistogramWidget.h>
#include <Modules/Math/BasicPlotter.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QtGui>

#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_point_data.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_text.h>
#include <qwt_symbol.h>
#include <qwt_math.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

BasicPlotterDialog::BasicPlotterDialog(const std::string& name, ModuleStateHandle state,
	QWidget* parent/* = 0*/)
	: ModuleDialogGeneric(state, parent)
{
	setupUi(this);
	setWindowTitle(QString::fromStdString(name));
	fixSize();

	addDoubleSpinBoxManager(verticalAxisSpinBox_, Parameters::VerticalAxisPosition);
  addDoubleSpinBoxManager(horizontalAxisSpinBox_, Parameters::HorizontalAxisPosition);
	//addCheckBoxManager(verticalAxisGroupBox_, Parameters::VerticalAxisVisible);
  //addCheckBoxManager(horizontalAxisGroupBox_, Parameters::HorizontalAxisVisible);
	addLineEditManager(titleLineEdit_, Parameters::PlotTitle);
	addLineEditManager(xAxisLineEdit_, Parameters::XAxisLabel);
	addLineEditManager(yAxisLineEdit_, Parameters::YAxisLabel);

	connect(showPlotPushButton_, SIGNAL(clicked()), this, SLOT(showPlot()));
}

BasicPlotterDialog::~BasicPlotterDialog()
{
	delete plotDialog_;
}

void BasicPlotterDialog::pullSpecial()
{
  auto data = transient_value_cast<DenseMatrixHandle>(state_->getTransientValue(Variables::InputMatrix));
	if (data)
		qDebug() << "received data matrix of size" << data->nrows() << "by" << data->ncols();
}

class Plot : public QwtPlot
{
public:
  explicit Plot( QWidget *parent = nullptr );
private:
  void populate();
};

void BasicPlotterDialog::showPlot()
{
	if (!plotDialog_)
	{
		plotDialog_ = new QDialog;
		plotDialog_->setStyleSheet(styleSheet());
		auto layout = new QHBoxLayout( plotDialog_ );
		layout->setContentsMargins( 5, 5, 5, 5 );
		auto plot = new Plot(this);
		layout->addWidget( plot );
		plotDialog_->resize( 600, 400 );
	}

	plotDialog_->show();
	plotDialog_->raise();
}

class FunctionData: public QwtSyntheticPointData
{
public:
    FunctionData( double( *y )( double ) ):
        QwtSyntheticPointData( 100 ),
        d_y( y )
    {
    }

    virtual double y( double x ) const
    {
        return d_y( x );
    }

private:
    double( *d_y )( double );
};

Plot::Plot( QWidget *parent ):
    QwtPlot( parent )
{
    setAutoFillBackground( true );

    setTitle( "A Simple QwtPlot Demonstration" );
    insertLegend( new QwtLegend(), QwtPlot::RightLegend );

    // axes
    setAxisTitle( xBottom, "x -->" );
    setAxisScale( xBottom, 0.0, 10.0 );

    setAxisTitle( yLeft, "y -->" );
    setAxisScale( yLeft, -1.0, 1.0 );

    // canvas
    QwtPlotCanvas *canvas = new QwtPlotCanvas(this);
    canvas->setLineWidth( 1 );
    canvas->setFrameStyle( QFrame::Box | QFrame::Plain );
    canvas->setBorderRadius( 15 );

    //QPalette canvasPalette( Qt::white );
    //canvasPalette.setColor( QPalette::Foreground, QColor( 133, 190, 232 ) );
    //canvas->setPalette( canvasPalette );

    setCanvas( canvas );

    // panning with the left mouse button
    ( void ) new QwtPlotPanner( canvas );

    // zoom in/out with the wheel
    ( void ) new QwtPlotMagnifier( canvas );

    populate();
}

void Plot::populate()
{
    // Insert new curves
    QwtPlotCurve *cSin = new QwtPlotCurve( "y = sin(x)" );
    cSin->setRenderHint( QwtPlotItem::RenderAntialiased );
    cSin->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
    cSin->setPen( Qt::red );
    cSin->attach( this );

    QwtPlotCurve *cCos = new QwtPlotCurve( "y = cos(x)" );
    cCos->setRenderHint( QwtPlotItem::RenderAntialiased );
    cCos->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
    cCos->setPen( Qt::blue );
    cCos->attach( this );

    // Create sin and cos data
    cSin->setData( new FunctionData( ::sin ) );
    cCos->setData( new FunctionData( ::cos ) );

    // Insert markers

    //  ...a horizontal line at y = 0...
    QwtPlotMarker *mY = new QwtPlotMarker();
    mY->setLabel( QString::fromLatin1( "y = 0" ) );
    mY->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
    mY->setLineStyle( QwtPlotMarker::HLine );
    mY->setYValue( 0.0 );
    mY->attach( this );

    //  ...a vertical line at x = 2 * pi
    QwtPlotMarker *mX = new QwtPlotMarker();
    mX->setLabel( QString::fromLatin1( "x = 2 pi" ) );
    mX->setLabelAlignment( Qt::AlignLeft | Qt::AlignBottom );
    mX->setLabelOrientation( Qt::Vertical );
    mX->setLineStyle( QwtPlotMarker::VLine );
    mX->setLinePen( Qt::black, 0, Qt::DashDotLine );
    mX->setXValue( 2.0 * M_PI );
    mX->attach( this );
}

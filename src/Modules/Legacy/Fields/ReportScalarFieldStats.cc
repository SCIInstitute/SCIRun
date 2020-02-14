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


///@author
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  February 2001

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

#include <string>
#include <vector>

#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

/// @class ReportScalarFieldStats
/// @brief Analyze data from a scalarfield.

class SCISHARE ReportScalarFieldStats : public Module
{
  public:
    ReportScalarFieldStats(GuiContext* ctx);
    virtual ~ReportScalarFieldStats() {}
    virtual void execute();

    void fill_histogram( std::vector<int>& hits);
    void clear_histogram();
    void local_reset_vars(){ reset_vars(); }

  private:
    GuiDouble min_;
    GuiDouble max_;
    GuiDouble mean_;
    GuiDouble median_;
    GuiDouble sigma_;   //standard deviation

    GuiInt is_fixed_;
    GuiInt nbuckets_;

};


DECLARE_MAKER(ReportScalarFieldStats)

ReportScalarFieldStats::ReportScalarFieldStats(GuiContext* ctx)
  : Module("ReportScalarFieldStats", ctx, Filter, "MiscField", "SCIRun"),
    min_(get_ctx()->subVar("min"), 0.0),
    max_(get_ctx()->subVar("max"), 0.0),
    mean_(get_ctx()->subVar("mean"), 0.0),
    median_(get_ctx()->subVar("median"), 0.0),
    sigma_(get_ctx()->subVar("sigma"),0.0),
    is_fixed_(get_ctx()->subVar("is_fixed"), 0),
    nbuckets_(get_ctx()->subVar("nbuckets"), 256)
{
}


void
ReportScalarFieldStats::clear_histogram()
{
  TCLInterface::execute(get_id() + " clear_data");
}


void
ReportScalarFieldStats::fill_histogram( std::vector<int>& hits)
{
  std::ostringstream ostr;
  int nmin, nmax;
  std::vector<int>::iterator it = hits.begin();
  nmin = 0;  nmax = *it;
  ostr << *it;  ++it;

  for(; it != hits.end(); ++it)
  {
    ostr <<" "<<*it;
    nmin = ((nmin < *it) ? nmin : *it );
    nmax = ((nmax > *it) ? nmax : *it );
  }
  ostr <<std::ends;
  std::string smin( to_string(nmin) );
  std::string smax( to_string(nmax) );

  std::string data = ostr.str();
  TCLInterface::execute(get_id() + " graph_data " + smin + " "
	       + smax + " " + data );
}


void
ReportScalarFieldStats::execute()
{
  // Get input field.
  FieldHandle ifield_handle;
  get_input_handle("Input Field", ifield_handle, true);

  if (!(ifield_handle->vfield()->is_scalar()))
  {
    error("This module only works on scalar fields.");
    return;
  }

  update_state(Executing);

  VField* ifield = ifield_handle->vfield();

  bool init = false;
  double value = 0;
  double min = 0;
  double max = 0;
  int counter = 0;
  std::vector<double> values;

  update_progress(0.3);
  double mean = 0;
  double mmin = min_.get();
  double mmax = max_.get();

  if ( is_fixed_.get() == 1 )
  {
    VField::size_type num_values = ifield->num_values();
    for (VField::index_type idx=0; idx < num_values ;idx++)
    {
      double val;
      ifield->get_value(val, idx);
      if ( val >= mmin && val <= mmax )
      {
        values.push_back( val );
        value += val;
        ++counter;
      }
    }

    mean = value/double(counter);
    mean_.set( mean );
  }
  else
  {
    VField::size_type num_values = ifield->num_values();
    for (VField::index_type idx=0; idx < num_values ;idx++)
    {
      double val;
      ifield->get_value(val, idx);
      values.push_back( val );
      value += val;
      if ( !init )
      {
        min = max = val;
        init = true;
      }
      else
      {
        min = (val < min) ? val:min;
        max = (val > max) ? val:max;
      }
      ++counter;
    }
    mean = value/double(counter);
    mean_.set( mean );

    min_.set( double( min ) );
    max_.set( double( max ) );
  }

  update_progress(0.6);

  local_reset_vars();
  if ((max_.get() - min_.get()) > 1e-16 && values.size() > 0)
  {
    int nbuckets = nbuckets_.get();
    std::vector<int> hits(nbuckets, 0);

    double frac = 1.0;
    frac = (nbuckets-1)/(max_.get() - min_.get());

    double sigma = 0.0;
    std::vector<double>::iterator vit = values.begin();
    std::vector<double>::iterator vit_end = values.end();
    for(; vit != vit_end; ++vit)
    {
      if( *vit >= min_.get() && *vit <= max_.get())
      {
        double value = (*vit - min_.get())*frac;
        hits[int(value)]++;
      }
      sigma += (*vit - mean)*(*vit - mean);
    }
    sigma_.set( sqrt( sigma / double(values.size()) ));

    vit = values.begin();
    nth_element(vit, vit+values.size()/2, vit_end);
    median_.set( double ( values[ values.size()/2] ) );
    fill_histogram( hits );
  }
  else
  {
    warning("min - max < precision or no values in range; clearing histogram");
    clear_histogram();
  }
}

} // End namespace SCIRun

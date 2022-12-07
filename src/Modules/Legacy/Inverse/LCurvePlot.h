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


///  File       : LCurvePlot.h
///  Author     : Jaume Coll-Font, Moritz Dannhauer, Ayla Khan, Dan White, Jess Tate
///  Date       : Oct 25th, 2017 (last update)

#ifndef MODULES_LEGACY_INVERSE_LCURVEPLOT_H__
#define MODULES_LEGACY_INVERSE_LCURVEPLOT_H__
#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Inverse/share.h>

namespace SCIRun {
namespace Modules {
namespace Inverse {

	class SCISHARE LCurvePlot
	{
	public:
		LCurvePlot();

    std::string update_lcurve_gui(const std::string& module_id,
      const SCIRun::Core::Datatypes::DenseMatrixHandle& lambda,
      const SCIRun::Core::Datatypes::DenseMatrixHandle& input,
      const SCIRun::Core::Datatypes::DenseMatrixHandle& lambda_index);
		const std::vector<double>& cornerPlot() const { return cornerPlot_; }
	private:
		std::vector<double> cornerPlot_;
	};
}}}


#endif

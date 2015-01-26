/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

/// @todo Documentation Modules/Visualization/CreateBasicColorMap.cc

#include <Modules/Visualization/ShowColorMapModule.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/ColorMap.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

ShowColorMap::ShowColorMap() : Module(ModuleLookupInfo("ShowColorMap", "Visualization", "SCIRun"))
{
  INITIALIZE_PORT(ColorMapObject);
	INITIALIZE_PORT(GeometryOutput); 
}

void ShowColorMap::setStateDefaults()
{
  auto state = get_state();
  state->setValue(DisplaySide, std::string("Rainbow"));
	state->setValue(DisplayLength, );
	state->setValue(TextSize);
	state->setValue(TextColor);
	state->setValue(Labels);
	state->setValue(Scale);
	state->setValue(Units);
	state->setValue(SignificantDigits);
	state->setValue(AddExtraSpace);
}

void ShowColorMap::execute()
{
	ColorMapHandle cmHandle = 0;
	if (!get_input_handle("ColorMap", cmHandle, true)) return;

	if (inputs_changed_ ||
		!geometry_output_handle_.get_rep() ||
		gui_length_.changed(true) ||
		gui_side_.changed(true) ||
		gui_numlabels_.changed(true) ||
		gui_scale_.changed(true) ||
		gui_num_sig_digits_.changed(true) ||
		gui_units_.changed(true) ||
		gui_text_fontsize_.changed(true) ||
		gui_extra_padding_.changed(true) ||
		color_changed_ == true)
	{
		// Inform module that execution started
		update_state(Executing);

		GeomGroup *all = new GeomGroup();

		Point  ref1(0.0, 0.0, 0.0);
		Vector out(0.0, 0.0, 0.0);
		Vector along(0.0, 0.0, 0.0);

		color_changed_ = false;

		text_material_handle_->diffuse =
			Color(gui_color_r_.get(), gui_color_g_.get(), gui_color_b_.get());

		if (gui_side_.get() == "left")
		{
			out = Vector(-0.05, 0.0, 0.0);
			if (gui_length_.get() == "full" || gui_length_.get() == "half2")
			{
				if (gui_extra_padding_.get())
				{
					ref1 = Point(-1.0, -14.0 / 16.0, 0.0);
				}
				else
				{
					ref1 = Point(-1.0, -15.0 / 16.0, 0.0);
				}
			}
			else
			{
				ref1 = Point(-1.0, 1.0 / 16.0, 1.0);
			}

			if (gui_length_.get() == "full")
			{
				if (gui_extra_padding_.get())
				{
					along = Vector(0.0, 29.0 / 16.0, 0.0);
				}
				else
				{
					along = Vector(0.0, 30.0 / 16.0, 0.0);
				}
			}
			else
			{
				along = Vector(0.0, 14.0 / 16.0, 0.0);
			}
		}
		else if (gui_side_.get() == "bottom")
		{
			out = Vector(0.0, -0.05, 0.0);
			if (gui_length_.get() == "full" || gui_length_.get() == "half1")
			{
				ref1 = Point(-15.0 / 16.0, -1.0, 0.0);
			}
			else
			{
				ref1 = Point(1.0 / 16.0, -1.0, 0.0);
			}

			if (gui_length_.get() == "full")
			{
				if (gui_extra_padding_.get())
				{
					along = Vector(29.0 / 16.0, 0.0, 0.0);
				}
				else
				{
					along = Vector(30.0 / 16.0, 0.0, 0.0);
				}
			}
			else
			{
				if (gui_extra_padding_.get())
				{
					along = Vector(13.0 / 16.0, 0.0, 0.0);
				}
				else
				{
					along = Vector(14.0 / 16.0, 0.0, 0.0);
				}
			}
		}

		const Point  ref0(ref1 - out);
		// Create a new colormap that we can send to ColorMapTex.  We need
		// to do this, because if the colormap min/max are too close you get
		// problems.  This is because the min and max are used to lookup
		// into the texture as floats.  Precion problems occur when the min
		// == max in floats, but not as doubles.
		ColorMapHandle cmap_rescaled;
		// Only rescale it when the min != max or min and max are too close.
		float too_close = Abs((float)(cmHandle->getMin()) - (float)(cmHandle->getMax()));
		// Replace zero compared with too_close with an epsilon if desired.
		if (too_close <= 0)
		{
			// Make a copy of the colormap we can rescale
			cmap_rescaled = cmHandle->clone();
			cmap_rescaled->Scale(0, 1);
		}
		else
		{
			cmap_rescaled = cmHandle;
		}

		ColorMapTex *sq = new ColorMapTex(ref0, ref0 + along,
			ref0 + along + out, ref0 + out, cmap_rescaled);
		all->add(sq);

		double scale = gui_scale_.get();
		std::string str = gui_units_.get();
		if (str.empty()) str = cmHandle->units();
		// So if the maximum number of digits the number will take up is
		// at most 25 then the length of str better be less than 80-25-1.
		// See size of value and num_sig_digits below.
		if (str.length() > 50)
		{
			error("Length of units string is too long.  Make it smaller than 50 characters please.");
			return;
		}

		const int numlabels = gui_numlabels_.get();
		if (numlabels > 1 && numlabels < 50)
		{
			// Fill in the text.
			const double minval = cmHandle->getMin()*scale;
			const double maxval = cmHandle->getMax()*scale;

			Point p0 = ref0 - out * 0.02;
			char value[80];
			GeomLines *lines = new GeomLines();
			GeomTexts *texts = new GeomTexts();
			texts->set_is_2d(true);
			texts->set_font_index(gui_text_fontsize_.get());
			int num_sig_digits = gui_num_sig_digits_.get();
			if (num_sig_digits < 1)
			{
				warning("Number of significant digits needs to be at least 1.  Setting the number of significant digits to 1.");
				gui_num_sig_digits_.set(1);
				num_sig_digits = 1;
			}
			if (num_sig_digits > 20)
			{
				warning("Number of significant digits needs to be less than or equal to 20.  Setting the number of significant digits to 20");
				gui_num_sig_digits_.set(20);
				num_sig_digits = 20;
			}
			for (int i = 0; i < numlabels; i++)
			{
				sprintf(value, "%.*g %s", num_sig_digits,
					minval + (maxval - minval)*(i / (numlabels - 1.0)),
					str.c_str());
				const Point loc = p0 + along * (i / (numlabels - 1.0));
				texts->add(value, loc, text_material_handle_->diffuse);
				lines->add(loc, text_material_handle_,
					loc + out * 0.5, text_material_handle_);
			}

			all->add(texts);
			all->add(lines);
		}

		GeomSticky *sticky = new GeomSticky(all);
		geometry_output_handle_ = GeomHandle(sticky);

		send_output_handle("Geometry", geometry_output_handle_, "ShowColorMap Sticky");
	}
  sendOutput(ColorMapObject, StandardColorMapFactory::create(get_state()->getValue(Variables::ColorMapName).toString()));
}

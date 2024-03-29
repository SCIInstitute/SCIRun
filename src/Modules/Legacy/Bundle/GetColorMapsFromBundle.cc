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


#include <Modules/Legacy/Bundle/GetColorMapsFromBundle.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/ColorMap.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

MODULE_INFO_DEF(GetColorMapsFromBundle, Bundle, SCIRun)

const AlgorithmParameterName GetColorMapsFromBundle::ColorMapNameList("ColorMapNameList");
const AlgorithmParameterName GetColorMapsFromBundle::ColorMapNames[] = {
  AlgorithmParameterName("colorMap1-name"),
  AlgorithmParameterName("colorMap2-name"),
  AlgorithmParameterName("colorMap3-name"),
  AlgorithmParameterName("colorMap4-name"),
  AlgorithmParameterName("colorMap5-name"),
  AlgorithmParameterName("colorMap6-name")
};

GetColorMapsFromBundle::GetColorMapsFromBundle() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputBundle);
  INITIALIZE_PORT(OutputBundle);
  INITIALIZE_PORT(colorMap1);
  INITIALIZE_PORT(colorMap2);
  INITIALIZE_PORT(colorMap3);
  INITIALIZE_PORT(colorMap4);
  INITIALIZE_PORT(colorMap5);
  INITIALIZE_PORT(colorMap6);
}

void GetColorMapsFromBundle::setStateDefaults()
{
  auto state = get_state();

  for (int i = 0; i < NUM_BUNDLE_OUT; ++i)
  {
    state->setValue(ColorMapNames[i], "colorMap" + std::to_string(i));
  }
}

void GetColorMapsFromBundle::execute()
{
  auto bundle = getRequiredInput(InputBundle);

  if (needToExecute())
  {
    auto state = get_state();
    state->setTransientValue(ColorMapNameList.name(), bundle->getColorMapNames());

    ColorMapHandle outputs[NUM_BUNDLE_OUT];
    for (int i = 0; i < NUM_BUNDLE_OUT; ++i)
    {
      auto colorMapName = state->getValue(ColorMapNames[i]).toString();
      if (bundle->isColorMap(colorMapName))
      {
        outputs[i] = bundle->getColorMap(colorMapName);;
      }
    }

    sendOutput(OutputBundle, bundle);

    //TODO: fix duplication
    if (outputs[0])
      sendOutput(colorMap1, outputs[0]);
    if (outputs[1])
      sendOutput(colorMap2, outputs[1]);
    if (outputs[2])
      sendOutput(colorMap3, outputs[2]);
    if (outputs[3])
      sendOutput(colorMap4, outputs[3]);
    if (outputs[4])
      sendOutput(colorMap5, outputs[4]);
    if (outputs[5])
      sendOutput(colorMap6, outputs[5]);
  }
}

std::string GetColorMapsFromBundle::makeColorMapNameList(const Bundle& bundle) const
{
  auto colorMapNames = bundle.getColorMapNames();
  std::ostringstream vars;
  std::copy(colorMapNames.begin(), colorMapNames.end(), std::ostream_iterator<std::string>(vars, "\n"));
  return vars.str();
}

#if 0
#include <Core/Datatypes/Bundle.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Dataflow/Network/Module.h>

#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/ColorMapPort.h>

using namespace SCIRun;

/// @class GetColorMapsFromBundle
/// @brief This module retrieves a colormap object from a bundle.
class GetColorMapsFromBundle : public Module {
public:
  GetColorMapsFromBundle(GuiContext*);
  virtual void execute();

private:
  GuiString             guicolormap1name_;
  GuiString             guicolormap2name_;
  GuiString             guicolormap3name_;
  GuiString             guicolormap4name_;
  GuiString             guicolormap5name_;
  GuiString             guicolormap6name_;
  GuiString             guicolormaps_;
};


DECLARE_MAKER(GetColorMapsFromBundle)
  GetColorMapsFromBundle::GetColorMapsFromBundle(GuiContext* ctx)
    : Module("GetColorMapsFromBundle", ctx, Filter, "Bundle", "SCIRun"),
      guicolormap1name_(get_ctx()->subVar("colormap1-name"), "colormap1"),
      guicolormap2name_(get_ctx()->subVar("colormap2-name"), "colormap2"),
      guicolormap3name_(get_ctx()->subVar("colormap3-name"), "colormap3"),
      guicolormap4name_(get_ctx()->subVar("colormap4-name"), "colormap4"),
      guicolormap5name_(get_ctx()->subVar("colormap5-name"), "colormap5"),
      guicolormap6name_(get_ctx()->subVar("colormap6-name"), "colormap6"),
      guicolormaps_(get_ctx()->subVar("colormap-selection",false), "")
{
}

void GetColorMapsFromBundle::execute()
{
  // Define input handle:
  BundleHandle handle;

  // Get data from input port:
  if (!(get_input_handle("bundle",handle,true))) return;

  if (inputs_changed_ || guicolormap1name_.changed() ||
      guicolormap2name_.changed() || guicolormap3name_.changed() ||
      guicolormap4name_.changed() || guicolormap5name_.changed() ||
      guicolormap6name_.changed() || !oport_cached("bundle") ||
      !oport_cached("colormap1")  || !oport_cached("colormap2") ||
      !oport_cached("colormap3")  || !oport_cached("colormap4") ||
      !oport_cached("colormap5")  || !oport_cached("colormap6"))
  {
    update_state(Executing);

    ColorMapHandle fhandle;
    std::string colormap1name = guicolormap1name_.get();
    std::string colormap2name = guicolormap2name_.get();
    std::string colormap3name = guicolormap3name_.get();
    std::string colormap4name = guicolormap4name_.get();
    std::string colormap5name = guicolormap5name_.get();
    std::string colormap6name = guicolormap6name_.get();
    std::string colormaplist;

    int numColorMaps = handle->numColorMaps();
    for (int p = 0; p < numColorMaps; p++)
    {
      colormaplist += "{" + handle->getColorMapName(p) + "} ";
    }

    guicolormaps_.set(colormaplist);
    get_ctx()->reset();

    // Send colormap1 if we found one that matches the name:
    if (handle->isColorMap(colormap1name))
    {
      fhandle = handle->getColorMap(colormap1name);
      send_output_handle("colormap1",fhandle);
    }

    // Send colormap2 if we found one that matches the name:
    if (handle->isColorMap(colormap2name))
    {
      fhandle = handle->getColorMap(colormap2name);
      send_output_handle("colormap2",fhandle);
    }

    // Send colormap3 if we found one that matches the name:
    if (handle->isColorMap(colormap3name))
    {
      fhandle = handle->getColorMap(colormap3name);
      send_output_handle("colormap3",fhandle);
    }

    // Send colormap4 if we found one that matches the name:
    if (handle->isColorMap(colormap4name))
    {
      fhandle = handle->getColorMap(colormap4name);
      send_output_handle("colormap4",fhandle);
    }

    // Send colormap5 if we found one that matches the name:
    if (handle->isColorMap(colormap5name))
    {
      fhandle = handle->getColorMap(colormap5name);
      send_output_handle("colormap5",fhandle);
    }

    // Send colormap6 if we found one that matches the name:
    if (handle->isColorMap(colormap6name))
    {
      fhandle = handle->getColorMap(colormap6name);
      send_output_handle("colormap6",fhandle);
    }

    send_output_handle("bundle",handle);
  }
}
#endif

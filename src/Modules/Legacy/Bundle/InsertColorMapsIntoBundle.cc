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


#include <Dataflow/Network/Module.h>

#include <Core/Datatypes/Bundle.h>

#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/ColorMapPort.h>

using namespace SCIRun;

/// @class InsertColorMapsIntoBundle
/// @brief This module inserts a colormap object into a bundle.

class InsertColorMapsIntoBundle : public Module {
public:
  InsertColorMapsIntoBundle(GuiContext*);
  virtual void execute();

private:
  GuiString     guicolormap1name_;
  GuiString     guicolormap2name_;
  GuiString     guicolormap3name_;
  GuiString     guicolormap4name_;
  GuiString     guicolormap5name_;
  GuiString     guicolormap6name_;

  GuiInt        guireplace1_;
  GuiInt        guireplace2_;
  GuiInt        guireplace3_;
  GuiInt        guireplace4_;
  GuiInt        guireplace5_;
  GuiInt        guireplace6_;

  GuiString     guibundlename_;
};


DECLARE_MAKER(InsertColorMapsIntoBundle)

InsertColorMapsIntoBundle::InsertColorMapsIntoBundle(GuiContext* ctx)
  : Module("InsertColorMapsIntoBundle", ctx, Filter, "Bundle", "SCIRun"),
    guicolormap1name_(get_ctx()->subVar("colormap1-name"), "colormap1"),
    guicolormap2name_(get_ctx()->subVar("colormap2-name"), "colormap2"),
    guicolormap3name_(get_ctx()->subVar("colormap3-name"), "colormap3"),
    guicolormap4name_(get_ctx()->subVar("colormap4-name"), "colormap4"),
    guicolormap5name_(get_ctx()->subVar("colormap5-name"), "colormap5"),
    guicolormap6name_(get_ctx()->subVar("colormap6-name"), "colormap6"),
    guireplace1_(get_ctx()->subVar("replace1"),1),
    guireplace2_(get_ctx()->subVar("replace2"),1),
    guireplace3_(get_ctx()->subVar("replace3"),1),
    guireplace4_(get_ctx()->subVar("replace4"),1),
    guireplace5_(get_ctx()->subVar("replace5"),1),
    guireplace6_(get_ctx()->subVar("replace6"),1),
    guibundlename_(get_ctx()->subVar("bundlename"), "")
{
}

void InsertColorMapsIntoBundle::execute()
{
  BundleHandle  handle;
  ColorMapHandle colormap1, colormap2, colormap3;
  ColorMapHandle colormap4, colormap5, colormap6;

  get_input_handle("bundle",handle,false);
  get_input_handle("colormap1",colormap1,false);
  get_input_handle("colormap2",colormap2,false);
  get_input_handle("colormap3",colormap3,false);
  get_input_handle("colormap4",colormap4,false);
  get_input_handle("colormap5",colormap5,false);
  get_input_handle("colormap6",colormap6,false);

  if (inputs_changed_ || guicolormap1name_.changed() ||
      guicolormap2name_.changed() || guicolormap3name_.changed() ||
      guicolormap4name_.changed() || guicolormap5name_.changed() ||
      guicolormap6name_.changed() ||
      guireplace1_.changed() || guireplace2_.changed() ||
      guireplace3_.changed() || guireplace4_.changed() ||
      guireplace5_.changed() || guireplace6_.changed() ||
      guibundlename_.changed() || !oport_cached("bundle"))
  {
    update_state(Executing);

    std::string colormap1Name = guicolormap1name_.get();
    std::string colormap2Name = guicolormap2name_.get();
    std::string colormap3Name = guicolormap3name_.get();
    std::string colormap4Name = guicolormap4name_.get();
    std::string colormap5Name = guicolormap5name_.get();
    std::string colormap6Name = guicolormap6name_.get();

    std::string bundlename = guibundlename_.get();

    if (handle.get_rep())
    {
      handle.detach();
    }
    else
    {
      handle = new Bundle();
      if (handle.get_rep() == 0)
      {
        error("Could not allocate new bundle");
        return;
      }
    }

    if (colormap1.get_rep()
        &&(guireplace1_.get()||!(handle->isColorMap(colormap1Name))))
      handle->setColorMap(colormap1Name,colormap1);

    if (colormap2.get_rep()
        &&(guireplace2_.get()||!(handle->isColorMap(colormap2Name))))
      handle->setColorMap(colormap2Name,colormap2);

    if (colormap3.get_rep()
        &&(guireplace3_.get()||!(handle->isColorMap(colormap3Name))))
      handle->setColorMap(colormap3Name,colormap3);

    if (colormap4.get_rep()
        &&(guireplace4_.get()||!(handle->isColorMap(colormap4Name))))
      handle->setColorMap(colormap4Name,colormap4);

    if (colormap5.get_rep()
        &&(guireplace5_.get()||!(handle->isColorMap(colormap5Name))))
      handle->setColorMap(colormap5Name,colormap5);

    if (colormap6.get_rep()
        &&(guireplace6_.get()||!(handle->isColorMap(colormap6Name))))
      handle->setColorMap(colormap6Name,colormap6);

    if (bundlename != "")
    {
      handle->set_property("name",bundlename,false);
    }

    send_output_handle("bundle",handle);
  }
}

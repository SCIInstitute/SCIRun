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


#include <Core/Datatypes/Bundle.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>

using namespace SCIRun;

/// @class InsertBundlesIntoBundle
/// @brief This module inserts a bundle object into a bundle.

class InsertBundlesIntoBundle : public Module {
public:
  InsertBundlesIntoBundle(GuiContext*);
  virtual void execute();

private:
  GuiString     guiBundle1Name_;
  GuiString     guiBundle2Name_;
  GuiString     guiBundle3Name_;
  GuiString     guiBundle4Name_;
  GuiString     guiBundle5Name_;
  GuiString     guiBundle6Name_;

  GuiInt        guireplace1_;
  GuiInt        guireplace2_;
  GuiInt        guireplace3_;
  GuiInt        guireplace4_;
  GuiInt        guireplace5_;
  GuiInt        guireplace6_;

  GuiString     guiBundleName_;
};


DECLARE_MAKER(InsertBundlesIntoBundle)

InsertBundlesIntoBundle::InsertBundlesIntoBundle(GuiContext* ctx)
  : Module("InsertBundlesIntoBundle", ctx, Filter, "Bundle", "SCIRun"),
    guiBundle1Name_(get_ctx()->subVar("bundle1-name"), "bundle1"),
    guiBundle2Name_(get_ctx()->subVar("bundle2-name"), "bundle2"),
    guiBundle3Name_(get_ctx()->subVar("bundle3-name"), "bundle3"),
    guiBundle4Name_(get_ctx()->subVar("bundle4-name"), "bundle4"),
    guiBundle5Name_(get_ctx()->subVar("bundle5-name"), "bundle5"),
    guiBundle6Name_(get_ctx()->subVar("bundle6-name"), "bundle6"),
    guireplace1_(get_ctx()->subVar("replace1"),1),
    guireplace2_(get_ctx()->subVar("replace2"),1),
    guireplace3_(get_ctx()->subVar("replace3"),1),
    guireplace4_(get_ctx()->subVar("replace4"),1),
    guireplace5_(get_ctx()->subVar("replace5"),1),
    guireplace6_(get_ctx()->subVar("replace6"),1),
    guiBundleName_(get_ctx()->subVar("bundlename",false), "")
{
}


void InsertBundlesIntoBundle::execute()
{
  BundleHandle  handle, bundle1, bundle2, bundle3, bundle4, bundle5, bundle6;

  get_input_handle("bundle",handle,false);
  get_input_handle("bundle1",bundle1,false);
  get_input_handle("bundle2",bundle2,false);
  get_input_handle("bundle3",bundle3,false);
  get_input_handle("bundle4",bundle4,false);
  get_input_handle("bundle5",bundle5,false);
  get_input_handle("bundle6",bundle6,false);

  if (inputs_changed_ || guiBundle1Name_.changed() || guiBundle2Name_.changed() ||
      guiBundle3Name_.changed() || guiBundle4Name_.changed() ||
      guiBundle5Name_.changed() || guiBundle6Name_.changed() ||
      guireplace1_.changed() || guireplace2_.changed() ||
      guireplace3_.changed() || guireplace4_.changed() ||
      guireplace5_.changed() || guireplace6_.changed() ||
      guiBundleName_.changed() || !oport_cached("bundle"))
  {
    update_state(Executing);

    std::string bundle1Name = guiBundle1Name_.get();
    std::string bundle2Name = guiBundle2Name_.get();
    std::string bundle3Name = guiBundle3Name_.get();
    std::string bundle4Name = guiBundle4Name_.get();
    std::string bundle5Name = guiBundle5Name_.get();
    std::string bundle6Name = guiBundle6Name_.get();
    std::string bundleName = guiBundleName_.get();

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

    if (bundle1.get_rep()
        &&(guireplace1_.get()||!(handle->isBundle(bundle1Name))))
      handle->setBundle(bundle1Name,bundle1);

    if (bundle2.get_rep()
        &&(guireplace2_.get()||!(handle->isBundle(bundle2Name))))
      handle->setBundle(bundle2Name,bundle2);

    if (bundle3.get_rep()
        &&(guireplace3_.get()||!(handle->isBundle(bundle3Name))))
      handle->setBundle(bundle3Name,bundle3);

    if (bundle4.get_rep()
        &&(guireplace4_.get()||!(handle->isBundle(bundle4Name))))
      handle->setBundle(bundle4Name,bundle4);

    if (bundle5.get_rep()
        &&(guireplace5_.get()||!(handle->isBundle(bundle5Name))))
      handle->setBundle(bundle5Name,bundle5);

    if (bundle6.get_rep()
        &&(guireplace6_.get()||!(handle->isBundle(bundle6Name))))
      handle->setBundle(bundle6Name,bundle6);

    if (bundleName != "")
    {
      handle->set_property("name",bundleName,false);
    }

    send_output_handle("bundle", handle);
  }
}

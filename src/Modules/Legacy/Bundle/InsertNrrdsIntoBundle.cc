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
#include <Core/Datatypes/NrrdData.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/NrrdPort.h>

using namespace SCIRun;

/// @class InsertNrrdsIntoBundle
/// @brief This module inserts a nrrd object into a bundle.

class InsertNrrdsIntoBundle : public Module {
public:
  InsertNrrdsIntoBundle(GuiContext*);
  virtual void execute();

private:
  GuiString     guinrrd1name_;
  GuiString     guinrrd2name_;
  GuiString     guinrrd3name_;
  GuiString     guinrrd4name_;
  GuiString     guinrrd5name_;
  GuiString     guinrrd6name_;

  GuiInt        guireplace1_;
  GuiInt        guireplace2_;
  GuiInt        guireplace3_;
  GuiInt        guireplace4_;
  GuiInt        guireplace5_;
  GuiInt        guireplace6_;

  GuiString     guibundlename_;
};


DECLARE_MAKER(InsertNrrdsIntoBundle)

InsertNrrdsIntoBundle::InsertNrrdsIntoBundle(GuiContext* ctx)
  : Module("InsertNrrdsIntoBundle", ctx, Filter, "Bundle", "SCIRun"),
    guinrrd1name_(get_ctx()->subVar("nrrd1-name"), "nrrd1"),
    guinrrd2name_(get_ctx()->subVar("nrrd2-name"), "nrrd2"),
    guinrrd3name_(get_ctx()->subVar("nrrd3-name"), "nrrd3"),
    guinrrd4name_(get_ctx()->subVar("nrrd4-name"), "nrrd4"),
    guinrrd5name_(get_ctx()->subVar("nrrd5-name"), "nrrd5"),
    guinrrd6name_(get_ctx()->subVar("nrrd6-name"), "nrrd6"),
    guireplace1_(get_ctx()->subVar("replace1"),1),
    guireplace2_(get_ctx()->subVar("replace2"),1),
    guireplace3_(get_ctx()->subVar("replace3"),1),
    guireplace4_(get_ctx()->subVar("replace4"),1),
    guireplace5_(get_ctx()->subVar("replace5"),1),
    guireplace6_(get_ctx()->subVar("replace6"),1),
    guibundlename_(get_ctx()->subVar("bundlename"), "")
{
}

void
InsertNrrdsIntoBundle::execute()
{
  BundleHandle  handle;
  NrrdDataHandle nrrd1, nrrd2, nrrd3;
  NrrdDataHandle nrrd4, nrrd5, nrrd6;

  get_input_handle("bundle",handle,false);
  get_input_handle("nrrd1",nrrd1,false);
  get_input_handle("nrrd2",nrrd2,false);
  get_input_handle("nrrd3",nrrd3,false);
  get_input_handle("nrrd4",nrrd4,false);
  get_input_handle("nrrd5",nrrd5,false);
  get_input_handle("nrrd6",nrrd6,false);

  if (inputs_changed_ || guinrrd1name_.changed() ||
      guinrrd2name_.changed() || guinrrd3name_.changed() ||
      guinrrd4name_.changed() || guinrrd5name_.changed() ||
      guinrrd6name_.changed() ||
      guireplace1_.changed() || guireplace2_.changed() ||
      guireplace3_.changed() || guireplace4_.changed() ||
      guireplace5_.changed() || guireplace6_.changed() ||

      guibundlename_.changed() || !oport_cached("bundle"))
  {
    update_state(Executing);

    std::string nrrd1Name = guinrrd1name_.get();
    std::string nrrd2Name = guinrrd2name_.get();
    std::string nrrd3Name = guinrrd3name_.get();
    std::string nrrd4Name = guinrrd4name_.get();
    std::string nrrd5Name = guinrrd5name_.get();
    std::string nrrd6Name = guinrrd6name_.get();
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

    if (nrrd1.get_rep()
        &&(guireplace1_.get()||!(handle->isNrrd(nrrd1Name))))
      handle->setNrrd(nrrd1Name,nrrd1);

    if (nrrd2.get_rep()
        &&(guireplace2_.get()||!(handle->isNrrd(nrrd2Name))))
      handle->setNrrd(nrrd2Name,nrrd2);

    if (nrrd3.get_rep()
        &&(guireplace3_.get()||!(handle->isNrrd(nrrd3Name))))
      handle->setNrrd(nrrd3Name,nrrd3);

    if (nrrd4.get_rep()
        &&(guireplace4_.get()||!(handle->isNrrd(nrrd4Name))))
      handle->setNrrd(nrrd4Name,nrrd4);

    if (nrrd5.get_rep()
        &&(guireplace5_.get()||!(handle->isNrrd(nrrd5Name))))
      handle->setNrrd(nrrd5Name,nrrd5);

    if (nrrd6.get_rep()
        &&(guireplace6_.get()||!(handle->isNrrd(nrrd6Name))))
      handle->setNrrd(nrrd6Name,nrrd6);

    if (bundlename != "")
    {
      handle->set_property("name",bundlename,false);
    }

    send_output_handle("bundle",handle);
  }
}

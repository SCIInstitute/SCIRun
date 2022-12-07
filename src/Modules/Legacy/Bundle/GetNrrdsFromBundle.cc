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

/// @class GetNrrdsFromBundle
/// @brief This module retrieves a nrrd object from a bundle.

class GetNrrdsFromBundle : public Module {
public:
  GetNrrdsFromBundle(GuiContext*);
  virtual void execute();

private:
  GuiString             guinrrd1name_;
  GuiString             guinrrd2name_;
  GuiString             guinrrd3name_;
  GuiString             guinrrd4name_;
  GuiString             guinrrd5name_;
  GuiString             guinrrd6name_;
  GuiInt                guitransposenrrd1_;
  GuiInt                guitransposenrrd2_;
  GuiInt                guitransposenrrd3_;
  GuiInt                guitransposenrrd4_;
  GuiInt                guitransposenrrd5_;
  GuiInt                guitransposenrrd6_;
  GuiString             guinrrds_;
};


DECLARE_MAKER(GetNrrdsFromBundle)

GetNrrdsFromBundle::GetNrrdsFromBundle(GuiContext* ctx)
  : Module("GetNrrdsFromBundle", ctx, Filter, "Bundle", "SCIRun"),
    guinrrd1name_(get_ctx()->subVar("nrrd1-name"), "nrrd1"),
    guinrrd2name_(get_ctx()->subVar("nrrd2-name"), "nrrd2"),
    guinrrd3name_(get_ctx()->subVar("nrrd3-name"), "nrrd3"),
    guinrrd4name_(get_ctx()->subVar("nrrd4-name"), "nrrd4"),
    guinrrd5name_(get_ctx()->subVar("nrrd5-name"), "nrrd5"),
    guinrrd6name_(get_ctx()->subVar("nrrd6-name"), "nrrd6"),
    guitransposenrrd1_(get_ctx()->subVar("transposenrrd1"), 0),
    guitransposenrrd2_(get_ctx()->subVar("transposenrrd2"), 0),
    guitransposenrrd3_(get_ctx()->subVar("transposenrrd3"), 0),
    guitransposenrrd4_(get_ctx()->subVar("transposenrrd4"), 0),
    guitransposenrrd5_(get_ctx()->subVar("transposenrrd5"), 0),
    guitransposenrrd6_(get_ctx()->subVar("transposenrrd6"), 0),
    guinrrds_(get_ctx()->subVar("nrrd-selection",false), "")
{
}


void
GetNrrdsFromBundle::execute()
{
  BundleHandle handle;

  // Get data from input port:
  get_input_handle("bundle",handle,true);

  if (inputs_changed_ || guinrrd1name_.changed() ||
      guinrrd2name_.changed() || guinrrd3name_.changed() ||
      guinrrd4name_.changed() || guinrrd5name_.changed() ||
      guinrrd6name_.changed() || guitransposenrrd1_.changed() ||
      guitransposenrrd2_.changed() || guitransposenrrd3_.changed() ||
      guitransposenrrd4_.changed() || guitransposenrrd5_.changed() ||
      guitransposenrrd6_.changed() || !oport_cached("bundle") ||
      !oport_cached("nrrd1") || !oport_cached("nrrd2") ||
      !oport_cached("nrrd3") || !oport_cached("nrrd4") ||
      !oport_cached("nrrd5") || !oport_cached("nrrd6"))
  {
    update_state(Executing);
    NrrdDataHandle fhandle;

    std::string nrrd1name = guinrrd1name_.get();
    std::string nrrd2name = guinrrd2name_.get();
    std::string nrrd3name = guinrrd3name_.get();
    std::string nrrd4name = guinrrd4name_.get();
    std::string nrrd5name = guinrrd5name_.get();
    std::string nrrd6name = guinrrd6name_.get();
    int transposenrrd1 = guitransposenrrd1_.get();
    int transposenrrd2 = guitransposenrrd2_.get();
    int transposenrrd3 = guitransposenrrd3_.get();
    int transposenrrd4 = guitransposenrrd4_.get();
    int transposenrrd5 = guitransposenrrd5_.get();
    int transposenrrd6 = guitransposenrrd6_.get();
    std::string nrrdlist;

    int numNrrds = handle->numNrrds();
    for (int p = 0; p < numNrrds; p++)
    {
      nrrdlist += "{" + handle->getNrrdName(p) + "} ";
    }

    guinrrds_.set(nrrdlist);
    get_ctx()->reset();

    // We need to set bundle properties hence we need to detach
    handle.detach();

    // Send nrrd1 if we found one that matches the name:
    if (handle->isNrrd(nrrd1name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd1) handle->transposeNrrd(true);
      fhandle = handle->getNrrd(nrrd1name);
      send_output_handle("nrrd1",fhandle);
    }

    // Send nrrd2 if we found one that matches the name:
    if (handle->isNrrd(nrrd2name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd2) handle->transposeNrrd(true);
      fhandle = handle->getNrrd(nrrd2name);
      send_output_handle("nrrd2",fhandle);
    }

    // Send matrix3 if we found one that matches the name:
    if (handle->isNrrd(nrrd3name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd3) handle->transposeNrrd(true);
      fhandle = handle->getNrrd(nrrd3name);
      send_output_handle("nrrd3",fhandle);
    }

    // Send nrrd4 if we found one that matches the name:
    if (handle->isNrrd(nrrd4name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd4) handle->transposeNrrd(true);
      fhandle = handle->getNrrd(nrrd4name);
      send_output_handle("nrrd4",fhandle);
    }

    // Send nrrd5 if we found one that matches the name:
    if (handle->isNrrd(nrrd5name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd5) handle->transposeNrrd(true);
      fhandle = handle->getNrrd(nrrd5name);
      send_output_handle("nrrd5",fhandle);
    }

    // Send matrix6 if we found one that matches the name:
    if (handle->isNrrd(nrrd6name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd6) handle->transposeNrrd(true);
      fhandle = handle->getNrrd(nrrd6name);
      send_output_handle("nrrd6",fhandle);
    }

    send_output_handle("bundle",handle);
  }
}

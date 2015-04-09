/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

/// @class ReportBundleInfo
/// @brief This module lists all the objects stored in a bundle. 

class ReportBundleInfo : public Module {
public:
  ReportBundleInfo(GuiContext*);
  virtual void execute();

private:
  GuiString tclInfoString_;
};


DECLARE_MAKER(ReportBundleInfo)

ReportBundleInfo::ReportBundleInfo(GuiContext* ctx)
  : Module("ReportBundleInfo", ctx, Sink, "Bundle", "SCIRun"),
    tclInfoString_(get_ctx()->subVar("tclinfostring",false), "")
{
}

void ReportBundleInfo::execute()
{
  /// Define the dataflow object
  BundleHandle bundle;

  /// Get the bundle from the module
  get_input_handle("bundle",bundle,true);

  /// If the input changed we need to update output
  if (inputs_changed_)
  {
    update_state(Executing);
    std::string tclinfostring;
   
    /// Get number of objects in bundle
    int numhandles = bundle->getNumHandles();

    std::string name;
    std::string type;
      
    /// Loop through all objects and display name and type
    for (int p=0; p < numhandles; p++)
    {
      name = bundle->getHandleName(p);
      type = bundle->getHandleType(p);
      tclinfostring += " {" + name + " (" + type + ") }";
    }
      
    tclInfoString_.set(tclinfostring);
  }
}


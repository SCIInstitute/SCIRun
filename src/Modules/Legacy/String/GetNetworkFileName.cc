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
/// @todo Documentation Modules/Legacy/String/GetNetworkFileName.cc
#if 0
#include <Core/Util/Environment.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/StringPort.h>

namespace SCIRun {

using namespace SCIRun;

class GetNetworkFileName : public Module {
public:
  GetNetworkFileName(GuiContext*);
  virtual void execute();
};


DECLARE_MAKER(GetNetworkFileName)

GetNetworkFileName::GetNetworkFileName(GuiContext* ctx) :
  Module("GetNetworkFileName", ctx, Source, "String", "SCIRun")
{
}


void
GetNetworkFileName::execute()
{
  std::string srn = "new.srn";
  if (sci_getenv_p("SCIRUN_NETFILE")) 
  {
    srn = sci_getenv("SCIRUN_NETFILE");
    if (srn.find("MyNetwork.srn") != std::string::npos)
      remark("GetNetworkFileName likely outputting filename of the last backed up .srn file, rather than the currently saved active network.");
  }

  StringHandle srnfn = new String(srn);
  send_output_handle("String",srnfn,true);
}


} // End namespace SCIRun


#endif
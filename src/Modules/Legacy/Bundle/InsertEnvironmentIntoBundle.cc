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

#include <Core/Util/Environment.h>
#include <Core/Datatypes/Bundle.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>

namespace SCIRun {

using namespace SCIRun;

/// @class InsertEnvironmentIntoBundle 
/// @brief Collects the current environment variables into a bundle. 

class InsertEnvironmentIntoBundle : public Module {
public:
  InsertEnvironmentIntoBundle(GuiContext*);

  virtual void execute();
};


DECLARE_MAKER(InsertEnvironmentIntoBundle)

InsertEnvironmentIntoBundle::InsertEnvironmentIntoBundle(GuiContext* ctx) :
  Module("InsertEnvironmentIntoBundle", ctx, Source, "Bundle", "SCIRun")
{
}

void
InsertEnvironmentIntoBundle::execute()
{
  update_state(Executing);

  std::map<std::string,std::string>& environment = get_sci_environment();

  std::map<std::string,std::string>::iterator it, it_end;
  it = environment.begin();
  it_end = environment.end();
  
  BundleHandle bundle = new Bundle();
  
  while (it != it_end)
  {
    std::string key = (*it).first;
    StringHandle data = new String((*it).second);
    bundle->setString(key,data);
    ++it;
  }
  
  send_output_handle("Environment",bundle,true);
}

} // End namespace SCIRun



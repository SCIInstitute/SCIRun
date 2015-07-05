/*  
 *  For more information, please see: http://software.sci.utah.edu
 *  
 *  The MIT License
 *  
 *  Copyright (c) 2015 Scientific Computing and Imaging Institute,
 *  University of Utah.
 *  
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */
 
#include <Core/Datatypes/String.h>

#include <Dataflow/Network/Ports/StringPort.h>
#include <Dataflow/Network/Module.h>

#include <sstream>
#include <vector>

namespace SCIRun {

using namespace SCIRun;

/// @class JoinStrings
/// This module merges multiple strings into one string. 

class JoinStrings : public Module {
  public:
    JoinStrings(GuiContext*);
    virtual ~JoinStrings() {}

    virtual void execute();
};


DECLARE_MAKER(JoinStrings)
JoinStrings::JoinStrings(GuiContext* ctx)
  : Module("JoinStrings", ctx, Source, "String", "SCIRun")
{
}


void
JoinStrings::execute()
{
  std::vector<StringHandle> strings;

  get_dynamic_input_handles("Input", strings, true);
  
  if (strings.size())
  {
    std::ostringstream ostr;
    for (size_t i = 0; i < strings.size(); ++i)
    {
      ostr << strings[i]->get();
    }
    StringHandle output(new String(ostr.str()));
    send_output_handle("Output", output);
  }
}

} // End namespace SCIRun



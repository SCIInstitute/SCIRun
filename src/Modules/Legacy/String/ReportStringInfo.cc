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

#include <Modules/Legacy/String/ReportStringInfo.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::StringManip;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

const ModuleLookupInfo ReportStringInfo::staticInfo_("ReportStringInfo", "String", "SCIRun");

ReportStringInfo::ReportStringInfo()
  : Module(staticInfo_)
{
}

#if 0
#include <Core/Datatypes/String.h>
#include <Dataflow/Network/Ports/StringPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

using namespace SCIRun;

/// @class ReportStringInfo
/// @brief This module can be used to display the contents of a string. 

class ReportStringInfo : public Module {
public:
  ReportStringInfo(GuiContext*);

  virtual ~ReportStringInfo();

  virtual void execute();

private:
  GuiString  inputstring_;
};



DECLARE_MAKER(ReportStringInfo)
ReportStringInfo::ReportStringInfo(GuiContext* ctx)
  : Module("ReportStringInfo", ctx, Source, "String", "SCIRun"),
    inputstring_(get_ctx()->subVar("inputstring"), "")
{
}

ReportStringInfo::~ReportStringInfo()
{
}

void
ReportStringInfo::execute()
{
  StringHandle handle;
  if (!get_input_handle("Input", handle, false))
  {
    inputstring_.set("<empty string>");
    return;
  }
  else
  {
    inputstring_.set(handle->get());
  }

  TCLInterface::execute(get_id() + " update");
}

} // End namespace SCIRun


#endif
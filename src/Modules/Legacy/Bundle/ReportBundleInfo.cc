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


#include <Modules/Legacy/Bundle/ReportBundleInfo.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Matrix.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Bundles;

/// @class ReportBundleInfo
/// @brief This module lists all the objects stored in a bundle.

MODULE_INFO_DEF(ReportBundleInfo, Bundle, SCIRun)

ReportBundleInfo::ReportBundleInfo() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputBundle);
}

void ReportBundleInfo::execute()
{
  auto bundle = getRequiredInput(InputBundle);

  if (needToExecute())
  {
    std::ostringstream infostring;

    for (const auto& nameHandlePair : *bundle)
    {
      std::string name = nameHandlePair.first;
      infostring << " {" << name << " (";
      auto obj = nameHandlePair.second;
      if (obj)
      {
        auto str = boost::dynamic_pointer_cast<Core::Datatypes::String>(obj);
        if (str)
        {
          infostring << str->value();
        }
        else
        {
          auto mat = boost::dynamic_pointer_cast<Core::Datatypes::Matrix>(obj);
          if (mat)
            infostring << "Matrix (" << mat->nrows() << "x" << mat->ncols() << ")";
          else
          {
            auto field = boost::dynamic_pointer_cast<Field>(obj);
            if (field)
              infostring << "Field (" << field->dynamic_type_name() << ")";
          }
        }
      }
      else
        infostring << "null";

      infostring << ") }\n";
    }

    get_state()->setTransientValue("ReportedInfo", infostring.str());
  }
}

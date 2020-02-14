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


/// @todo Documentation Core/Utils/Legacy/Environment_Defaults.cc

#include <Core/Utils/Legacy/Environment.h>

#include <map>

namespace SCIRun {

  void create_environment_defaults(std::map<std::string, std::string>& envMap)
  {
#ifdef _WIN32
    envMap["SCIRUN_TMP_DIR"] = "C:/Windows/Temp";
    envMap["SCIRUN_SERV_TMP_DIR"] = "C:/Windows/Temp";
#else
    envMap["SCIRUN_TMP_DIR"] = "/tmp";
    envMap["SCIRUN_SERV_TMP_DIR"] = "/tmp";
#endif
    envMap["SCIRUN_GUI_UseGuiFetch"] = "off";
    envMap["SCIRUN_GUI_MoveGuiToMouse"] = "off";

    envMap["SCIRUN_STARTMATLAB"] = "matlab -nojvm -nodesktop";
    envMap["SCIRUN_MATLABTIMEOUT"] = "180";
  }

}

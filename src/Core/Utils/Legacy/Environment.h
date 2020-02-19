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


///
///@file  Environment.h
///@brief Interface to setting environemnt variables and parsing .rc files
///
///@author
///       McKay Davis
///       Scientific Computing and Imaging Institute
///       University of Utah
///@date  January 2004
///

#ifndef Core_Util_Environment_h
#define Core_Util_Environment_h 1

#include <string>
#include <map>
#include <Core/Utils/Legacy/share.h>

namespace SCIRun {

  SCISHARE void create_environment_defaults(std::map<std::string, std::string>& envMap);
  SCISHARE void create_sci_environment(char **env, const char *execname);
  SCISHARE void find_and_parse_rcfile(const std::string &rcfile);
  SCISHARE bool parse_rcfile(const char* file);
  SCISHARE void copy_and_parse_scirunrc();
  SCISHARE bool update_rcfile(const std::string& key, const std::string& value);

  // Use the following functions to get/put environment variables.
  SCISHARE void sci_putenv( const std::string & key, const std::string & val );
  // Returns NULL if 'key' not found.
  SCISHARE const char *sci_getenv( const std::string & key );

  // sci_getenv_p
  // will return a bool representing the value of environment variable 'key'
  // returns FALSE if and only if:
  //   the variable does not exist,
  //   or is empty,
  //   or is equal (Case insensitive) to 'false', 'no', 'off', or '0'
  // returns TRUE:
  //   otherwise.
  SCISHARE bool sci_getenv_p( const std::string & key );

  SCISHARE const std::map<std::string,std::string>& get_sci_environment();

  SCISHARE bool replace_environment_variables(std::string& str);
  SCISHARE std::string get_example_nets_dir(const std::string& objdir);

} // end namespace SCIRun

#endif // #ifndef Core_Util_Environment_h

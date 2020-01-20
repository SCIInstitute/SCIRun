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


/*
 *  FieldIEPlugin:  Data structure needed to make a SCIRun FieldIE Plugin
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   May 2004
 *
 */

#ifndef SCI_project_FieldIEPlugin_h
#define SCI_project_FieldIEPlugin_h 1

#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/ImportExport/GenericIEPlugin.h>
#include <Core/ImportExport/share.h>

namespace SCIRun
{
  typedef GenericIEPluginInterface<Field> FieldIEPlugin;

  typedef IEPluginLegacyAdapter<Field> FieldIEPluginLegacyAdapter;

  class SCISHARE FieldIEPluginManager : public GenericIEPluginManager<Field>
  {

  };

  /// @TODO: blank class needed for Windows export lib...otherwise library is all template now
  class SCISHARE IEPluginManagerManager
  {
  public:
    IEPluginManagerManager();
  };

}

#endif

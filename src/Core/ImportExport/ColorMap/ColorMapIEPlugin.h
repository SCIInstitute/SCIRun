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
 *  ColorMapIEPlugin:  Data structure needed to make a SCIRun ColorMapIE Plugin
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   May 2004
 *
 */

#ifndef SCI_project_ColorMapIEPlugin_h
#define SCI_project_ColorMapIEPlugin_h 1

#include <Core/ImportExport/GenericIEPlugin.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/ImportExport/share.h>

namespace SCIRun
{

  typedef GenericIEPluginInterface<Core::Datatypes::ColorMap> ColorMapIEPlugin;

  typedef IEPluginLegacyAdapter<Core::Datatypes::ColorMap> ColorMapIEPluginLegacyAdapter;

  typedef GenericIEPluginManager<Core::Datatypes::ColorMap> ColorMapIEPluginManager;

}

#endif

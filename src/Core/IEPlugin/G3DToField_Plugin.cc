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
 * Implementation notes:
 *
 * This reader does not read textures, just geometry.
 *
 */

#include <Core/IEPlugin/G3DToField_Plugin.h>
#include <Core/Algorithms/Legacy/DataIO/G3DToFieldReader.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Logging/Log.h>
#include <Core/IEPlugin/IEPluginInit.h>

using namespace SCIRun;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;

FieldHandle SCIRun::G3DToField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle result;
  G3DToFieldReader reader(pr);
  std::string fn(filename);
  if (! reader.read(fn, result))
  {
    if (pr) pr->error("Convert g3d to field failed.");
    return (result);
  }

  return result;
}

bool SCIRun::FieldToG3D_writer(LoggerHandle pr, FieldHandle fh, const char* filename)
{
  G3DToFieldReader writer(pr);

  if(!writer.write(std::string(filename), fh))
  {
    if (pr) pr->error("Converting field to g3d failed.");
    return false;
  }

  return true;
}

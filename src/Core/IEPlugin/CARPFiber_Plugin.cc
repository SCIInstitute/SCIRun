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


#include <Core/IEPlugin/CARPFiber_Plugin.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/IEPlugin/TriSurfField_Plugin.h>
#include <Core/IEPlugin/TetVolField_Plugin.h>
#include <Core/Utils/Legacy/StringUtil.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/lexical_cast.hpp>


using namespace std;
using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

FieldHandle SCIRun::CARPFiber_reader(LoggerHandle pr, const char *filename)
{
  return TextToTetVolField_reader(pr, filename);
}

bool SCIRun::CARPFiber_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
{
  if (!fh)
    return false;

  VMesh *mesh = fh->vmesh();
  VField *field = fh->vfield();

  FieldInformation fi(fh);

   // Isotropic Fiber file
  {
    std::ofstream outputfile;
    outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    std::string lon_fn(filename);
    std::string::size_type pos = lon_fn.find_last_of(".");
    std::string base = lon_fn.substr(0, pos);
    std::string ext  = lon_fn.substr(pos);
    const char* fileExt = ".lon";

    if (pos == std::string::npos)
    {
      lon_fn += fileExt;
    }
    else if (ext != fileExt)
    {
      lon_fn = base + fileExt;
    }

    try
    {
      outputfile.open(lon_fn.c_str());

      VMesh::Cell::iterator cellIter;
      VMesh::Cell::iterator cellIterEnd;
      VMesh::Cell::size_type cellSize;

      mesh->begin(cellIter);
      mesh->end(cellIterEnd);
      mesh->size(cellSize);

#if DEBUG
      std::cerr << "Number of tets = " << cellSize << std::endl;
      std::cout << *cellIterEnd << std::endl;
#endif

      if (fi.is_tensor())
      {
        outputfile << 2 << std::endl;
        Tensor tensor;
        for (VMesh::index_type idx = 0; idx < *cellIterEnd; idx++)
        {
          field->get_value(tensor, idx);
          outputfile << tensor.val(0, 0) << " " << tensor.val(0, 1) << " " << tensor.val(0, 2) << " " << tensor.val(1, 1) << " " << tensor.val(1, 2) << " " << tensor.val(2, 2) << '\n';
        }
      }
      else if (fi.is_vector())
      {
        outputfile << 1 << std::endl;
        Vector val;
        for (VMesh::index_type idx = 0; idx < *cellIterEnd; idx++)
        {
          field->get_value(val, idx);
          outputfile << val.x() << " " << val.y() << " " << val.z() << '\n';
        }
      }
      else
      {
        if (pr) pr->error("Please convert to Tetvol");
        return false;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write to file: " + lon_fn);
      return false;
    }
    outputfile.close();
  }


  return true;
}

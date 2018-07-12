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

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/IEPlugin/CARPSurf_Plugin.h>
#include <Core/Utils/Legacy/StringUtil.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

FieldHandle SCIRun::CARPSurf_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle result = 0;

  std::string fac_fn(filename);
  std::string pts_fn(filename);

  // Check whether the .fac or .tri file exists
  std::string::size_type pos = fac_fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    fac_fn = fac_fn + ".elem";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
      inputfile.open(fac_fn.c_str());
    }

    catch (...)
    {
      if (pr) pr->error("Could not open file: "+fac_fn);
      return (result);
    }
  }
  else
  {
    std::string base = fac_fn.substr(0,pos);
    std::string ext  = fac_fn.substr(pos);
    if ((ext != ".elem" )||(ext != ".surf"))
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        fac_fn = base + ".elem";
        inputfile.open(fac_fn.c_str());
      }
      catch (...)
      {
        try
        {
          std::ifstream inputfile;
          inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
          fac_fn = base + ".surf";
          inputfile.open(fac_fn.c_str());
        }

        catch (...)
        {
          if (pr) pr->error("Could not open file: "+base + ".elem");
          return (result);
        }
      }
    }
    else
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        inputfile.open(fac_fn.c_str());
      }

      catch (...)
      {
        if (pr) pr->error("Could not open file: "+fac_fn);
        return (result);
      }
    }
  }


  pos = pts_fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    pts_fn = pts_fn + ".pts";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
      inputfile.open(pts_fn.c_str());
    }

    catch (...)
    {
      if (pr) pr->error("Could not open file: "+pts_fn);
      return (result);
    }
  }
  else
  {
    std::string base = pts_fn.substr(0,pos);
    std::string ext  = pts_fn.substr(pos);
    if (ext != ".pts" )
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        pts_fn = base + ".pts";
        inputfile.open(pts_fn.c_str());
      }
      catch (...)
      {
          if (pr) pr->error("Could not open file: "+base + ".pts");
          return (result);
        }
      }
    else
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        inputfile.open(pts_fn.c_str());
      }

      catch (...)
      {
        if (pr) pr->error("Could not open file: "+pts_fn);
        return (result);
      }
    }
  }


  int ncols = 0;
  int nrows = 0;
  int line_ncols = 0;
  int num_elems = 0, num_nodes = 0;

  std::string line;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE NUMBER OF NODES
  // AND CHECK THE FILE'S INTEGRITY.

  bool first_line_pts = true;
  bool first_line = true;

  std::vector<double> values;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(pts_fn.c_str());

      while ( getline(inputfile,line,'\n') )
      {
      
        multiple_from_string(line,values);
        line_ncols = values.size();

        if (first_line_pts)
        {
          if (line_ncols > 0)
          {
            if (line_ncols == 1)
            {
              num_nodes =  static_cast<int>(values[0]) + 1;
            }
            else if ((line_ncols == 3)||(line_ncols == 2))
            {
              first_line_pts = false;
              nrows++;
              ncols = line_ncols;
            }
            else
            {
              if (pr)  pr->error("Improper format of text file, some lines contain more than 3 entries");
              return (result);
            }
          }
        }
        else
        {
          if (line_ncols > 0)
          {
            nrows++;
            if (ncols > 0)
            {
              if (ncols != line_ncols)
              {
                if (pr)  pr->error("Improper format of text file, not every line contains the same amount of coordinates");
                return (result);
              }
            }
            else
            {
              ncols = line_ncols;
            }
          }
        }
      }
    }
    catch (...) {
      if (pr) pr->error("Could not open file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  if (0 == num_nodes) {
    num_nodes = nrows;
  }

  nrows = 0;
  ncols = 0;
  line_ncols = 0;

  bool zero_based = false;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(fac_fn.c_str());

      while( getline(inputfile,line,'\n'))
      {

        multiple_from_string(line,values);
        line_ncols = values.size();

        for (size_t j=0; j<values.size(); j++) if (values[j] == 0.0) zero_based = true;

        if (first_line)
        {
          if (line_ncols > 0)
          {
            if (line_ncols == 1)
            {
              num_elems =  static_cast<int>(values[0]) + 1;
            }
            else if ((line_ncols == 4)||(line_ncols == 5))
            {
              first_line = false;
              nrows++;
              ncols = line_ncols;
            }
            else
            {
              if (pr)  pr->error("Improper format of text file, some lines do not contain 3 entries");
              return (result);
            }
          }
        }
        else
        {
          if (line_ncols > 0)
          {
            nrows++;
            if (ncols > 0)
            {
              if (ncols != line_ncols)
              {
                if (pr)  pr->error("Improper format of text file, not every line contains the same amount of coordinates");
                return (result);
              }
            }
            else
            {
              ncols = line_ncols;
            }
          }
        }
      }
    }
    catch (...) {
      if (pr) pr->error("Could not open file: " + fac_fn);
      return (result);
    }
    inputfile.close();
  }

  if (0 == num_elems)
  {
    num_elems = nrows;
  }

  FieldInformation fi("TriSurfMesh", 1,"double");
  result = CreateField(fi);

  VMesh *mesh = result->vmesh();

  mesh->node_reserve(num_nodes);
  mesh->elem_reserve(num_elems);

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(pts_fn.c_str());

      std::vector<double> vdata(3);

      for(int i = 0; i < num_nodes && getline(inputfile,line,'\n'); ++i)
      {

        multiple_from_string(line,values);

        if (values.size() == 3) mesh->add_point(Point(values[0]/1000,values[1]/1000,values[2]/1000));
        if (values.size() == 2) mesh->add_point(Point(values[0]/1000,values[1]/1000,0.0));
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: "+std::string(pts_fn));
      return (result);
    }
    inputfile.close();
  }

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(fac_fn.c_str());

      VMesh::Node::array_type vdata;
      vdata.resize(3);

      std::vector<VMesh::index_type> ivalues;

      for (int i = 0; i < num_elems && getline(inputfile,line,'\n'); ++i)
      {

        multiple_from_string(line,ivalues);
        for (size_t j = 0; j < ivalues.size() && j < 3; j++)
        {
          if (zero_based) vdata[j] = ivalues[j];
          else vdata[j] = ivalues[j]-1;
        }

        if (ivalues.size() > 2) mesh->add_elem(vdata);
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: "+std::string(fac_fn));
      return (result);
    }
    inputfile.close();
  }

  return (result);
}




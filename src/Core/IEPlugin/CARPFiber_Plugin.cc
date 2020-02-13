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
  FieldHandle result;

  std::string elems_fn(filename);
  std::string pts_fn(filename);

  // Check whether the .elems or .tri file exists
  std::string::size_type pos = elems_fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    elems_fn = elems_fn + ".elem";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
      inputfile.open(elems_fn.c_str());
    }

    catch (...)
    {
      if (pr) pr->error("Could not open file: "+elems_fn);
      return (result);
    }
  }
  else
  {
    std::string base = elems_fn.substr(0,pos);
    std::string ext  = elems_fn.substr(pos);
    if ((ext != ".elem" ))
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        elems_fn = base + ".elem";
        inputfile.open(elems_fn.c_str());
      }
      catch (...)
      {
        try
        {
          std::ifstream inputfile;
          inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
          elems_fn = base + ".tet";
          inputfile.open(elems_fn.c_str());
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
        inputfile.open(elems_fn.c_str());
      }

      catch (...)
      {
        if (pr) pr->error("Could not open file: "+elems_fn);
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
    if ((ext != ".pts" )||(ext != ".pos"))
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
        try
        {
          std::ifstream inputfile;
          inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
          pts_fn = base + ".pos";
          inputfile.open(pts_fn.c_str());
        }

        catch (...)
        {
          if (pr) pr->error("Could not open file: "+base + ".pts");
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
  int num_nodes = 0, num_elems = 0;

  std::string line;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE NUMBER OF NODES
  // AND CHECK THE FILE'S INTEGRITY.

  bool has_header_pts = false;
  bool first_line_pts = true;

  bool has_header = false;
  bool first_line = true;

  bool has_data = false;

  std::vector<double> values;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(pts_fn.c_str());

      while (getline(inputfile,line,'\n'))
      {
        if (line.size() > 0)
        {
          // block out comments
          if ((line[0] == '#')||(line[0] == '%')) continue;
        }

        // replace comma's and tabs with white spaces
        for (size_t p = 0;p<line.size();p++)
        {
          if ((line[p] == '\t')||(line[p] == ',')||(line[p]=='"')) line[p] = ' ';
        }

        multiple_from_string(line,values);
        line_ncols = values.size();

        if (first_line_pts)
        {
          if (line_ncols > 0)
          {
            if (line_ncols == 1)
            {
              has_header_pts = true;
              num_nodes = static_cast<int>(values[0]) + 1;
            }
            else if ((line_ncols > 1))
            {
              has_header_pts = false;
              first_line_pts = false;
              nrows++;
              ncols = line_ncols;
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
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  if (0 == num_nodes)
  {
    num_nodes = nrows;
  }
  else if ( has_header_pts && (nrows != num_nodes) )
  {
    if (pr) pr->warning("Number of nodes listed in header (" + boost::lexical_cast<std::string>(num_nodes) +
                        ") does not match number of non-header rows in file (" + boost::lexical_cast<std::string>(nrows) + ")");
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
      inputfile.open(elems_fn.c_str());

      while (getline(inputfile,line,'\n'))
      {
        if (line.size() > 0)
        {
          // block out comments
          if ((line[0] == '#')||(line[0] == '%')) continue;
        }

        // replace comma's and tabs with white spaces
        for (size_t p = 0;p<line.size();p++)
        {
          if ((line[p] == '\t')||(line[p] == ',')||(line[p]=='"')) line[p] = ' ';
        }

        multiple_from_string(line,values);
        line_ncols = values.size();

        for (size_t j=0; j<values.size(); j++) if (values[j] == 0.0) zero_based = true;

        if (first_line)
        {
          if (line_ncols > 0)
          {
            if (line_ncols == 1)
            {
              has_header = true;
              num_elems = static_cast<int>(values[0]) + 1;
            }
            else if (line_ncols > 3)
            {
              has_header = false;
              first_line = false;
              nrows++;
              ncols = line_ncols;
              if (ncols == 5) has_data = true;
            }
            else
            {
              if (pr)  pr->error("Improper format of text file, some lines do not contain 4 entries");
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
                if (pr)  pr->error("Improper format of text file, not every line contains the same amount of node references");
                return (result);
              }
            }
            else
            {
              ncols = line_ncols;
              if (ncols == 5) has_data = true;
            }
          }
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + elems_fn);
      return (result);
    }
    inputfile.close();
  }

  if (0 == num_elems)
  {
    num_elems = nrows;
  }
  else if ( has_header && (nrows != num_elems) )
  {
    if (pr) pr->warning("Number of elements listed in header (" + boost::lexical_cast<std::string>(num_elems) +
                        ") does not match number of non-header rows in file (" + boost::lexical_cast<std::string>(nrows) + ")");
  }

  // add data to elems (constant basis)
  FieldInformation fi("TetVolMesh",-1,"double");
  if (has_data) fi.make_constantdata();
  result = CreateField(fi);

  VMesh *mesh = result->vmesh();
  VField *field = result->vfield();

  mesh->node_reserve(num_nodes);
  mesh->elem_reserve(num_elems);

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(pts_fn.c_str());

      std::vector<double> vdata(3);

      for (int i = 0; i < num_nodes && getline(inputfile,line,'\n'); ++i)
      {
        if (line.size() > 0)
        {
          // block out comments
          if ((line[0] == '#')||(line[0] == '%')) continue;
        }

        // replace comma's and tabs with white spaces
        for (size_t p = 0;p<line.size();p++)
        {
          if ((line[p] == '\t')||(line[p] == ',')||(line[p]=='"')) line[p] = ' ';
        }

        multiple_from_string(line,values);

        if (values.size() == 3) mesh->add_point(Point(values[0],values[1],values[2]));
        if (values.size() == 2) mesh->add_point(Point(values[0],values[1],0.0));
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and process file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  std::vector<double> fvalues;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(elems_fn.c_str());

      VMesh::Node::array_type vdata;
      vdata.resize(4);

      std::vector<VMesh::index_type> ivalues;

      for (int i = 0; i < num_elems && getline(inputfile,line,'\n'); ++i)
      {
        if (line.size() > 0)
        {
          // block out comments
          if ((line[0] == '#')||(line[0] == '%')) continue;
        }

        // replace comma's and tabs with white spaces
        for (size_t p = 0;p<line.size();p++)
        {
          if ((line[p] == '\t')||(line[p] == ',')||(line[p]=='"')) line[p] = ' ';
        }

        multiple_from_string(line,ivalues);
        for (size_t j=0; j<ivalues.size() && j<4; j++)
        {
          if (zero_based) vdata[j] = ivalues[j];
          else vdata[j] = ivalues[j]-1;
        }
        if (ivalues.size() > 4) fvalues.push_back(ivalues[4]);

        if (ivalues.size() > 3) mesh->add_elem(vdata);
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and process file: " + elems_fn);
      return (result);
    }
    inputfile.close();
  }

  if (has_data)
  {
    field->resize_values();
    field->set_values(fvalues);
  }
  return (result);
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

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


#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Util/StringUtil.h>

#include <sci_debug.h>

#include <iostream>
#include <fstream>
#include <sstream>

namespace SCIRun {

FieldHandle BYUToTriSurfField_reader(ProgressReporter *pr, const char *filename);

bool Dummy_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);

inline void convert_to_whitespace(std::string& line)
{
  /*if (line.size() > 0)
  {
    // block out comments
    if ((line[0] == '#')||(line[0] == '%')) continue;
  }
  */
  // replace comma's and tabs with white spaces
  for (size_t p = 0;p<line.size();p++)
  {
    if ((line[p] == '\t')||(line[p] == ',')||(line[p]=='"')) line[p] = ' ';
  }
}

FieldHandle BYUToTriSurfField_reader(ProgressReporter *pr, const char *filename)
{
  FieldHandle result = 0;

  std::string byu_fn(filename);

  // Check whether the .byu file exists
  std::string::size_type pos = byu_fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    byu_fn = byu_fn + ".byu";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
      inputfile.open(byu_fn.c_str());
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: " + byu_fn);
      return (result);
    }
  }
  else
  {
    std::string base = byu_fn.substr(0,pos);
    std::string ext  = byu_fn.substr(pos);
    if (ext != ".byu" )
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        byu_fn = base + ".byu";
        inputfile.open(byu_fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: " + base + ".byu");
        return (result);
      }
    }
    else
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        inputfile.open(byu_fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: " + byu_fn);
        return (result);
      }
    }
  }

  int nrows = 0;
  int line_ncols = 0;
  int head_count = 0;
  int part_num = 0;
  int vertex_num = 0;
  int poly_num = 0;
  int edge_num = 0;

  bool zero_based = false;

  std::string line;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE NUMBER OF NODES
  // AND CHECK THE FILE'S INTEGRITY.

  std::vector<double> values;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(byu_fn.c_str());

      while (getline(inputfile,line,'\n'))
      {
        convert_to_whitespace(line);
        multiple_from_string(line,values);
        line_ncols = values.size();

        if (line_ncols > 0)
        {
          //std::cout << line_ncols << std::endl << nrows << std::endl;
          if (line_ncols == 4 && nrows == 0)
          {
            head_count++;
            nrows++;
            //Read in header info from first line
            part_num = static_cast<int>(values[0]);
            vertex_num = static_cast<int>(values[1]);
            poly_num = static_cast<int>(values[2]);
            edge_num = static_cast<int>(values[3]);
            //std::cout << "Hello Line Zero!";

          }
          else if (line_ncols == 2 && nrows == 1)
          {
            head_count++;
            nrows++;
          }
          else if (line_ncols == 3 && nrows < vertex_num+2)
          {
            nrows++;
          }
          else if (line_ncols == 6 && nrows < vertex_num+2)
          {
            nrows++;
            nrows++;
          }
          else if(line_ncols == 3 && nrows >= vertex_num+2)
          {
            for (size_t j=0; j<values.size(); j++) if (values[j] == 0.0) zero_based = true;
          }
          else
          {
            if (pr) pr->error("Improper format of text file, unrecognize structure (error 1)");
            return (result);
          }
        }
        if (nrows > 1 && head_count < 2)
        {
          if (pr)  pr->error("Improper format of text file, does not contain proper header for .byu file type (error 2");
          return (result);
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + byu_fn);
      return (result);
    }
    inputfile.close();
  }
  int num_nodes = vertex_num;
  int num_elems = poly_num;

#if DEBUG
  std::cout << "num_nodes = " << num_nodes << std::endl;
  std::cout << "num_elems = " << num_elems << std::endl;
#endif

  FieldInformation fi("TriSurfMesh", 1,"double");
  result = CreateField(fi);

  VMesh *mesh = result->vmesh();

  mesh->node_reserve(num_nodes);
  mesh->elem_reserve(num_elems);

  nrows = 0;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(byu_fn.c_str());

      VMesh::Node::array_type vdata;
      vdata.resize(3);

      const int NUM_HEADER_LINES = 2;
      std::vector<double> pvalues;
      for (int i = 0; i < NUM_HEADER_LINES; i++)
      {
        if (! getline(inputfile,line,'\n'))
        {
          if (pr)  pr->error("Could not read header line (error 3");
          return (result);
        }
        // read in header
      }

      for (int i = 0; i < num_nodes; i++)
      {
        if (! getline(inputfile,line,'\n'))
        {
          if (pr)  pr->error("Could not read verticies line (error 4");
          return (result);
        }
        convert_to_whitespace(line);
        multiple_from_string(line,pvalues);
        // read in points and add to mesh
        if (pvalues.size() == 3)
        {
          mesh->add_point(Point(pvalues[0],pvalues[1],pvalues[2]));
        }

        else if (pvalues.size() == 6)
        {
          mesh->add_point(Point(pvalues[0],pvalues[1],pvalues[2]));
          mesh->add_point(Point(pvalues[3],pvalues[4],pvalues[5]));
          i++;
        }
      }

      std::vector<VMesh::index_type> ivalues;

      for (int i = 0; i < num_elems; i++)
      {
        if (! getline(inputfile,line,'\n'))
        {
          if (pr)  pr->error("Could not read element line (error 5");
          return (result);
        }
        convert_to_whitespace(line);
        multiple_from_string(line,ivalues);
        // read in elements and add to mesh
        if (ivalues.size() == 3)
        {
          if (zero_based)
          {
            vdata[0] = ivalues[0];
            vdata[1] = ivalues[1];
            vdata[2] = -1*ivalues[2];
            mesh->add_elem(vdata);
            nrows++;
          }
          else
          {
            vdata[0] = ivalues[0]-1;
            vdata[1] = ivalues[1]-1;
            vdata[2] = -1*ivalues[2]-1;
            mesh->add_elem(vdata);
            nrows++;
          }
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + byu_fn);
      return (result);
    }
    inputfile.close();
  }

  return (result);
}

bool Dummy_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
{
  return false;
}

static FieldIEPlugin BYUSurfField_plugin("BYUSurfField", "{.byu}", "", BYUToTriSurfField_reader, Dummy_writer);

} // end namespace

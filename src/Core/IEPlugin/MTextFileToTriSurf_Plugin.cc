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
 *  SimpleTextToTriSurf_IEPlugin.cc
 *
 *  Written by:
 *   Jeroen Stinstra
 *   Department of Computer Science
 *   University of Utah
 *
 */

#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/Field.h>
#include <Core/Util/StringUtil.h>

#include <iostream>
#include <fstream>

namespace SCIRun {

class Face {
  public:
    Mesh::index_type a,b,c;
};

FieldHandle MTextFileToTriSurf_reader(ProgressReporter *pr, const char *filename)
{
  FieldHandle result = 0;

  std::string fn(filename);

  // Check whether the .m file exists
  std::string::size_type pos = fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    fn = fn + ".m";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
      inputfile.open(fn.c_str());
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: " + fn);
      return (result);
    }
  }
  else
  {
    std::string base = fn.substr(0,pos);
    std::string ext  = fn.substr(pos);
    if (ext != ".m" )
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        fn = base + ".m";
        inputfile.open(fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: " + base + ".m");
        return (result);
      }
    }
    else
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        inputfile.open(fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: " + fn);
        return (result);
      }
    }
  }

  std::string line;
  std::vector<double> values;
  std::vector<VMesh::index_type> ivalues;

  Mesh::size_type nnodes = 0;
  Mesh::size_type nfaces = 0;
  // STAGE 1 - SCAN THE FILE TO DETERMINE THE NUMBER OF NODES
  // AND CHECK THE FILE'S INTEGRITY.

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(fn.c_str());

      while( getline(inputfile,line,'\n'))
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

        while(line[0] == ' ') line = line.substr(1);

        if (line.substr(0,6) == "Vertex")
        {
          VMesh::index_type idx = 0;

          multiple_from_string(line,values);

          idx = 0;
          if (values.size() > 0) idx = static_cast<VMesh::index_type>(values[0]);
          if (idx > nnodes) nnodes = idx;
        }
        else if (line.substr(0,4) == "Face")
        {
          VMesh::index_type idx = 0;

          multiple_from_string(line,ivalues);
          idx = 0;
          if (ivalues.size() > 0) idx = static_cast<VMesh::index_type>(ivalues[0]);
          if (idx > nfaces) nfaces = idx;
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + fn);
      return (result);
    }
    inputfile.close();
  }

  // Now create field
  FieldInformation fi("TriSurfMesh",-1,"double");
  result = CreateField(fi);

  VMesh *mesh = result->vmesh();
  VField *field = result->vfield();

  mesh->node_reserve(nnodes);
  mesh->elem_reserve(nfaces);

  std::vector<Point> nodes(nnodes);
  std::vector<VMesh::Node::array_type> faces(nfaces);

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(fn.c_str());

      while( getline(inputfile,line,'\n'))
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

        while(line[0] == ' ') line = line.substr(1);

        if (line.substr(0,6) == "Vertex")
        {
          multiple_from_string(line,values);

          Mesh::index_type idx = 0;
          double x,y,z;

          if (values.size() > 3)
          {
            idx = static_cast<VMesh::index_type>(values[0]);
            x = values[1]; y = values[2]; z = values[3];
            nodes[idx-1] = Point(x,y,z);
          }
        }
        else if (line.substr(0,4) == "Face")
        {
          multiple_from_string(line,ivalues);

          Mesh::index_type idx;
          Mesh::index_type a,b,c;

          if (ivalues.size() >3)
          {
            idx = static_cast<VMesh::index_type>(ivalues[0]);
            a = ivalues[1];
            b = ivalues[2];
            c = ivalues[3];

            faces[idx-1].resize(3);
            faces[idx-1][0] = a-1;
            faces[idx-1][1] = b-1;
            faces[idx-1][2] = c-1;
          }
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and process file: " + fn);
      return (result);
    }
    inputfile.close();
  }

  for (Mesh::index_type i=0;i<nnodes; i++) mesh->add_point(nodes[i]);
  for (Mesh::index_type i=0;i<nfaces; i++) mesh->add_elem(faces[i]);
  field->resize_fdata();

  return (result);
}

static FieldIEPlugin MFileToTriSurf_plugin("MFileToTriSurf","{.m}", "",MTextFileToTriSurf_reader,0);

} // end namespace

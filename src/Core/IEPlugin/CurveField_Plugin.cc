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
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Logging/LoggerInterface.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {

FieldHandle TextToCurveField_reader(LoggerHandle pr, const char *filename);

bool CurveFieldToTextBaseIndexZero_writer(LoggerHandle pr, FieldHandle fh, const char *filename);
bool CurveFieldToTextBaseIndexOne_writer(LoggerHandle pr, FieldHandle fh, const char *filename);

FieldHandle TextToCurveField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle result = 0;

  std::string pts_fn(filename);
  std::string edge_fn(filename);

  std::string::size_type pos = pts_fn.find_last_of(".");
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
      if (pr) pr->error("Could not open file: " + pts_fn);
      return (result);
    }
  }
  else
  {
    std::string base = pts_fn.substr(0,pos);
    std::string ext  = pts_fn.substr(pos);
    if (ext == ".pts" || ext == ".pos")
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        inputfile.open(pts_fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: " + pts_fn);
        return (result);
      }
    }
    else // try appending pts or pos extension
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
          if (pr) pr->error("Could not open file: " + base + ".pts");
          return (result);
        }
      }
    }
  }

  // Check whether the .edge file exists
  pos = edge_fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    edge_fn = edge_fn + ".edge";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
      inputfile.open(edge_fn.c_str());
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: " + edge_fn);
      return (result);
    }
  }
  else
  {
    std::string base = edge_fn.substr(0,pos);
    std::string ext  = edge_fn.substr(pos);
    if (ext != ".edge" )
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        edge_fn = base + ".edge";
        inputfile.open(edge_fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: " + base + ".edge");
        return (result);
      }
    }
    else
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        inputfile.open(edge_fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: " + edge_fn);
        return (result);
      }
    }
  }

  int ncols = 0;
  int nrows = 0;
  int line_ncols = 0;

  std::string line;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE NUMBER OF NODES
  // AND CHECK THE FILE'S INTEGRITY.

  bool has_header = false;
  bool first_line = true;

  std::vector<double> values;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(pts_fn.c_str());

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

        multiple_from_string(line,values);
        line_ncols = values.size();

        if (first_line)
        {
          if (ncols > 0)
          {
            if (line_ncols == 1)
            {
              has_header = true;
            }
            else if ((line_ncols == 3)||(line_ncols == 2))
            {
              has_header = false;
              first_line = false;
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
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  int num_nodes = nrows;

  nrows = 0;
  ncols = 0;
  line_ncols = 0;

  bool zero_based = false;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(edge_fn.c_str());

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

        multiple_from_string(line,values);
        line_ncols = values.size();

        for (size_t j=0; j<values.size(); j++) if (values[j] == 0.0) zero_based = true;
        if (first_line)
        {
          if (ncols > 0)
          {
            if (line_ncols == 1)
            {
              has_header = true;
            }
            else if (line_ncols > 1)
            {
              has_header = false;
              first_line = false;
              nrows++;
              ncols = line_ncols;
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
      if (pr) pr->error("Could not open and read file: " + edge_fn);
      return (result);
    }
    inputfile.close();
  }

  int num_elems = nrows;

  FieldInformation fi("CurveMesh", -1, "double");
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

        multiple_from_string(line,values);

        if (values.size() == 3) mesh->add_point(Point(values[0],values[1],values[2]));
        if (values.size() == 2) mesh->add_point(Point(values[0],values[1], 0.0));
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(edge_fn.c_str());

      VMesh::Node::array_type vdata;
      vdata.resize(2);

      std::vector<VMesh::index_type> ivalues;

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

        multiple_from_string(line,ivalues);
        for (size_t j=0; j<ivalues.size() && j<2; j++)
        {
          if (zero_based) vdata[j] = ivalues[j];
          else vdata[j] = ivalues[j]-1;
        }

        if (ivalues.size() > 1) mesh->add_elem(vdata);
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + edge_fn);
      return (result);
    }
    inputfile.close();
  }
  return (result);
}

bool CurveFieldToTextBaseIndexZero_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
{
  VMesh *mesh = fh->vmesh();

  // Points file
  {
    std::ofstream outputfile;
    outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    std::string pts_fn(filename);
    std::string::size_type pos = pts_fn.find_last_of(".");
    std::string base = pts_fn.substr(0, pos);
    std::string ext  = pts_fn.substr(pos);
    const char* fileExt = ".pts";

    if (pos == std::string::npos)
    {
      pts_fn += fileExt;
    }
    else if (ext != fileExt)
    {
      pts_fn = base + fileExt;
    }

    try
    {
      outputfile.open(pts_fn.c_str());

      // these appear to be reasonable formatting flags for output
      std::ios_base::fmtflags ff;
      ff = outputfile.flags();
      ff |= outputfile.showpoint; // write floating-point values including always the decimal point
      ff |= outputfile.fixed; // write floating point values in fixed-point notation
      outputfile.flags(ff);

      VMesh::Node::iterator nodeIter;
      VMesh::Node::iterator nodeIterEnd;
      VMesh::Node::size_type nodeSize;

      mesh->begin(nodeIter);
      mesh->end(nodeIterEnd);
      mesh->size(nodeSize);

     // N.B: not writing header

      while (nodeIter != nodeIterEnd)
      {
        Point p;
        mesh->get_center(p, *nodeIter);
        outputfile << p.x() << " " << p.y() << " " << p.z() << "\n";
        ++nodeIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write file: " + pts_fn);
      return (false);
    }
    outputfile.close();
  }

  // Edges file
  {
    std::ofstream outputfile;
    std::string edges_fn(filename);
    std::string::size_type pos = edges_fn.find_last_of(".");
    std::string base = edges_fn.substr(0, pos);
    std::string ext  = edges_fn.substr(pos);
    const char* fileExt = ".edge";

    if (pos == std::string::npos)
    {
      edges_fn += fileExt;
    }
    else if (ext != fileExt)
    {
      edges_fn = base + fileExt;
    }

    try
    {
      outputfile.open(edges_fn.c_str());

      VMesh::Edge::iterator edgeIter;
      VMesh::Edge::iterator edgeIterEnd;
      VMesh::Edge::size_type edgeSize;
      VMesh::Node::array_type edgeNodes(2);

      mesh->begin(edgeIter);
      mesh->end(edgeIterEnd);
      mesh->size(edgeSize);

      while (edgeIter != edgeIterEnd) {
        mesh->get_nodes(edgeNodes, *edgeIter);
        outputfile << edgeNodes[0] << " "
                   << edgeNodes[1] << "\n";
        ++edgeIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write file: " + edges_fn);
      return (false);
    }
    outputfile.close();
  }
  return true;
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
bool CurveFieldToTextBaseIndexOne_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
{
  VMesh *mesh = fh->vmesh();

  // Points file
  {
    std::ofstream outputfile;
    std::string pts_fn(filename);
    std::string::size_type pos = pts_fn.find_last_of(".");
    std::string base = pts_fn.substr(0, pos);
    std::string ext  = pts_fn.substr(pos);
    const char* fileExt = ".pts";

    if (pos == std::string::npos)
    {
      pts_fn += fileExt;
    }
    else
    {
      pts_fn = base + fileExt;
    }

    try
    {
      outputfile.open(pts_fn.c_str());

      // these appear to be reasonable formatting flags for output
      std::ios_base::fmtflags ff;
      ff = outputfile.flags();
      ff |= outputfile.showpoint; // write floating-point values including always the decimal point
      ff |= outputfile.fixed; // write floating point values in fixed-point notation
      outputfile.flags(ff);

      VMesh::Node::iterator nodeIter;
      VMesh::Node::iterator nodeIterEnd;
      VMesh::Node::size_type nodeSize;

      mesh->begin(nodeIter);
      mesh->end(nodeIterEnd);
      mesh->size(nodeSize);

     // N.B: not writing header

      while (nodeIter != nodeIterEnd)
      {
        Point p;
        mesh->get_center(p, *nodeIter);
        outputfile << p.x() << " " << p.y() << " " << p.z() << "\n";
        ++nodeIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write file: " + pts_fn);
      return (false);
    }
    outputfile.close();
  }

  // Edges file
  {
    std::ofstream outputfile;
    std::string edges_fn(filename);
    std::string::size_type pos = edges_fn.find_last_of(".");
    std::string base = edges_fn.substr(0, pos);
    std::string ext  = edges_fn.substr(pos);
    const char* fileExt = ".edge";

    if (pos == std::string::npos)
    {
      edges_fn += fileExt;
    }
    else
    {
      edges_fn = base + fileExt;
    }

    try
    {
      outputfile.open(edges_fn.c_str());

      VMesh::Edge::iterator edgeIter;
      VMesh::Edge::iterator edgeIterEnd;
      VMesh::Edge::size_type edgeSize;
      VMesh::Node::array_type edgeNodes(2);

      mesh->begin(edgeIter);
      mesh->end(edgeIterEnd);
      mesh->size(edgeSize);

      while (edgeIter != edgeIterEnd) {
        mesh->get_nodes(edgeNodes, *edgeIter);
        outputfile << edgeNodes[0] << " "
                   << edgeNodes[1] << "\n";
        ++edgeIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write file: " + edges_fn);
      return (false);
    }
    outputfile.close();
  }
  return true;
}
#endif

//static FieldIEPlugin CurveField_plugin("CurveField", "{.pts} {.pos} {.edge}", "", TextToCurveField_reader, CurveFieldToTextBaseIndexZero_writer);
//static FieldIEPlugin CurveFieldBaseIndexOne_plugin("CurveField[BaseIndex 1]", "{.pts} {.edge}", "", 0, CurveFieldToTextBaseIndexOne_writer);

} // end namespace

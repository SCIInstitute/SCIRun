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

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

namespace SCIRun {

FieldHandle TextToQuadSurfField_reader(ProgressReporter *pr, const char *filename);

bool QuadSurfFieldToTextBaseIndexZero_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);
bool QuadSurfFieldToTextBaseIndexOne_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);

FieldHandle TextToQuadSurfField_reader(ProgressReporter *pr, const char *filename)
{
  FieldHandle result = 0;

  std::string quad_fn(filename);
  std::string pts_fn(filename);


  // Check whether the .quad file exists
  std::string::size_type pos = quad_fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    quad_fn = quad_fn + ".quad";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
      inputfile.open(quad_fn.c_str());
    }

    catch (...)
    {
      if (pr) pr->error("Could not open file: "+quad_fn);
      return (result);
    }
  }
  else
  {
    std::string base = quad_fn.substr(0,pos);
    std::string ext  = quad_fn.substr(pos);
    if (ext != ".quad" )
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        quad_fn = base + ".quad";
        inputfile.open(quad_fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: "+base + ".fac");
        return (result);
      }
    }
    else
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        inputfile.open(quad_fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: "+quad_fn);
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

  bool has_header = false;
  bool first_line = true;
  bool first_line_elems = true;

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
          if (line_ncols > 0)
          {
            if (line_ncols == 1)
            {
              has_header = true;
              num_nodes = static_cast<int>(values[0]) + 1;
            }
            else if ((line_ncols == 3)||(line_ncols == 2))
            {
              has_header = false;
              first_line = false;
              ++nrows;
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
            ++nrows;
            if (ncols > 0)
            {
              if (ncols != line_ncols)
              {
                if (pr)  pr->error("Improper format of text file, not every line contains the same number of points");
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
  else if ( has_header && (num_nodes != nrows) )
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
      inputfile.open(quad_fn.c_str());

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

        if (first_line_elems)
        {
          if (line_ncols > 0)
          {
            if (line_ncols == 1)
            {
              has_header = true;
              num_elems = static_cast<int>(values[0]) + 1;
            }
            else if (line_ncols == 4)
            {
              has_header = false;
              first_line_elems = false;
              ++nrows;
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
            ++nrows;
            if (ncols > 0)
            {
              if (ncols != line_ncols)
              {
                if (pr)  pr->error("Improper format of text file, not every line contains the same number of coordinates");
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
      if (pr) pr->error("Could not open and read file: " + quad_fn);
      return (result);
    }
    inputfile.close();
  }

  if (0 == num_elems)
  {
    num_elems = nrows;
  }
  else if ( has_header && (num_elems != nrows) )
  {
    if (pr) pr->warning("Number of elements listed in header (" + boost::lexical_cast<std::string>(num_elems) +
                        ") does not match number of non-header rows in file (" + boost::lexical_cast<std::string>(nrows) + ")");
  }

  FieldInformation fi("QuadSurfMesh", "QuadBilinearLgn", "double");
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
      if (pr) pr->error("Could not open file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(quad_fn.c_str());

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
        if (ivalues.size() > 3) mesh->add_elem(vdata);
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: " + quad_fn);
      return (result);
    }
    inputfile.close();
  }
  return (result);
}

bool QuadSurfFieldToTextBaseIndexZero_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
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
      if (pr) pr->error("Could not open file: "+ pts_fn);
      return (false);
    }
    outputfile.close();
  }

  // Quads file
  {
    std::ofstream outputfile;
    outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    std::string quads_fn(filename);
    std::string::size_type pos = quads_fn.find_last_of(".");
    std::string base = quads_fn.substr(0, pos);
    std::string ext  = quads_fn.substr(pos);
    const char* fileExt = ".quad";

    if (pos == std::string::npos)
    {
      quads_fn += fileExt;
    }
    else if (ext != fileExt)
    {
      quads_fn = base + fileExt;
    }

    try
    {
      outputfile.open(quads_fn.c_str());

      VMesh::Face::iterator faceIter;
      VMesh::Face::iterator faceIterEnd;
      VMesh::Face::size_type faceSize;
      VMesh::Node::array_type faceNodes(4);

      mesh->begin(faceIter);
      mesh->end(faceIterEnd);
      mesh->size(faceSize);

      while (faceIter != faceIterEnd) {
        mesh->get_nodes(faceNodes, *faceIter);
        outputfile << faceNodes[0] << " "
                   << faceNodes[1] << " "
                   << faceNodes[2] << " "
                   << faceNodes[3] << "\n";
        ++faceIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: " + quads_fn);
      return (false);
    }
    outputfile.close();
  }
  return true;
}


bool QuadSurfFieldToTextBaseIndexOne_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
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
      if (pr) pr->error("Could not open file: " + pts_fn);
      return (false);
    }
    outputfile.close();
  }

  // Quads file
  {
    std::ofstream outputfile;
    outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    std::string quads_fn(filename);
    std::string::size_type pos = quads_fn.find_last_of(".");
    std::string base = quads_fn.substr(0, pos);
    std::string ext  = quads_fn.substr(pos);
    const char* fileExt = ".quad";

    if (pos == std::string::npos)
    {
      quads_fn += fileExt;
    }
    else
    {
      quads_fn = base + fileExt;
    }

    try
    {
      outputfile.open(quads_fn.c_str());

      VMesh::Face::iterator faceIter;
      VMesh::Face::iterator faceIterEnd;
      VMesh::Face::size_type faceSize;
      VMesh::Node::array_type faceNodes(4);

      mesh->begin(faceIter);
      mesh->end(faceIterEnd);
      mesh->size(faceSize);

      int baseIndex = 1;

      while (faceIter != faceIterEnd) {
        mesh->get_nodes(faceNodes, *faceIter);
        outputfile << faceNodes[0] + baseIndex << " "
                   << faceNodes[1] + baseIndex << " "
                   << faceNodes[2] + baseIndex << " "
                   << faceNodes[3] + baseIndex << "\n";
        ++faceIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: " + quads_fn);
      return (false);
    }
    outputfile.close();
  }
  return true;
}


static FieldIEPlugin QuadSurfField_plugin("QuadSurfField", "{.quad} {.pts} {.pos}", "", TextToQuadSurfField_reader, QuadSurfFieldToTextBaseIndexZero_writer);
static FieldIEPlugin QuadSurfFieldBaseIndexOne_plugin("QuadSurfField[BaseIndex 1]", "{.quad} {.pts}", "", 0, QuadSurfFieldToTextBaseIndexOne_writer);

} // end namespace

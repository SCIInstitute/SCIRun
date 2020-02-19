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


#include <Core/IEPlugin/CARPMesh_Plugin.h>
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

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

FieldHandle SCIRun::CARPMesh_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle result;

  std::string elems_fn(filename);
  std::string pts_fn(filename);

  // Check whether the .elem file exists
  std::string::size_type pos = elems_fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    elems_fn = elems_fn + ".elem";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      inputfile.open(elems_fn.c_str());
    }

    catch (...)
    {
      if (pr) pr->error("Could not open file: " + elems_fn);
      return (result);
    }
  }
  else
  {
    std::string base = elems_fn.substr(0, pos);
    std::string ext = elems_fn.substr(pos);
    if ((ext != ".elem"))
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        elems_fn = base + ".elem";
        inputfile.open(elems_fn.c_str());
      }
      catch (...)
      {
        try
        {
          std::ifstream inputfile;
          inputfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
          elems_fn = base + ".tet";
          inputfile.open(elems_fn.c_str());
        }
        catch (...)
        {
          if (pr) pr->error("Could not open file: " + base + ".elem");
          return (result);
        }
      }
    }
    else
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        inputfile.open(elems_fn.c_str());
      }

      catch (...)
      {
        if (pr) pr->error("Could not open file: " + elems_fn);
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
      inputfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
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
    std::string base = pts_fn.substr(0, pos);
    std::string ext = pts_fn.substr(pos);
    if ((ext != ".pts") || (ext != ".pos"))
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        pts_fn = base + ".pts";
        inputfile.open(pts_fn.c_str());
      }
      catch (...)
      {
        try
        {
          std::ifstream inputfile;
          inputfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
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
    else
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        inputfile.open(pts_fn.c_str());
      }

      catch (...)
      {
        if (pr) pr->error("Could not open file: " + pts_fn);
        return (result);
      }
    }
  }

  std::string line;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE NUMBER OF NODES
  // AND CHECK THE FILE'S INTEGRITY.

  int num_nodes = 0; int num_elems = 0;
  std::vector<double> values;
  std::vector<double> fvalues;
  std::vector<VMesh::index_type> ivalues;
  std::string elem_type;


  // Check the element type

  {
    std::ifstream inputfile;
    inputfile.exceptions(std::ifstream::badbit);

    try
    {
      inputfile.open(elems_fn.c_str());

      for (int lineno = 0; getline(inputfile, line) && lineno < 2; lineno++)
      {
        if (lineno != 0)
        {
          for (size_t k = 0; k < line.size(); k++)
          {
            if (line[k] == ' ') { break; }
            else { elem_type += line[k]; }
          }
        }
      }

      if ((elem_type != "Tt") && (elem_type != "Tr"))
      {
        if (pr) pr->error("Mesh types other than Tet and Tri not supported");
        return (result);
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not read element type");
      return (result);
    }
  }

  // add data to elems (constant basis)

//std::cout << "Loading element type: " << std::endl;
//std::cout << elem_type << std::endl;

  FieldInformation fi(nullptr);

  int elem_n = (elem_type == "Tt") ? 4 : 3;

  if (elem_type == "Tt")
  {
    fi = FieldInformation("TetVolMesh", -1, "double");
    fi.make_constantdata();

  }
  else if (elem_type == "Tr")
  {
    fi = FieldInformation("TriSurfMesh", -1, "double");

  }

  result = CreateField(fi);

  VMesh *mesh = result->vmesh();
  VField *field = result->vfield();

  // Elements file

  {
    std::ifstream inputfile;
    inputfile.exceptions(std::ifstream::badbit);

    try
    {
      inputfile.open(elems_fn.c_str());

      VMesh::Node::array_type vdata;
      vdata.resize(elem_n);

      getline(inputfile, line, '\n');
      multiple_from_string(line, values);
      num_elems = static_cast<int>(values[0]);
      mesh->elem_reserve(num_elems);

      for (int i = 0; i < num_elems && getline(inputfile, line, '\n'); ++i)
      {

        multiple_from_string(line, ivalues);

        for (size_t j = 0; j < ivalues.size() && j < elem_n; j++)
        {
          vdata[j] = ivalues[j];
        }

        fvalues.push_back(ivalues[ivalues.size() - 1]);

        mesh->add_elem(vdata);

      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and process file: " + elems_fn);
      return (result);
    }
    inputfile.close();
  }


  // Points file

  {
    std::ifstream inputfile;
    inputfile.exceptions(std::ifstream::badbit);

    try
    {
      inputfile.open(pts_fn.c_str());

      std::vector<double> vdata(3);
      getline(inputfile, line, '\n');
      multiple_from_string(line, values);
      num_nodes = static_cast<int>(values[0]);

      for (int i = 0; i < num_nodes && getline(inputfile, line, '\n'); ++i) {

        multiple_from_string(line, values);
        int sf = 1000;

        if (values.size() == 3) mesh->add_point(Point(values[0] / sf, values[1] / sf, values[2] / sf));

      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and process file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  field->resize_values();
  field->set_values(fvalues);

  return (result);
}

bool SCIRun::CARPMesh_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
{
  if (!fh)
    return false;

  VMesh *mesh = fh->vmesh();
  VField *field = fh->vfield();

  FieldInformation fi(fh);


  // Points file
  {
    std::ofstream outputfile;
    outputfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    std::string pts_fn(filename);
    std::string::size_type pos = pts_fn.find_last_of(".");
    std::string base = pts_fn.substr(0, pos);
    std::string ext = pts_fn.substr(pos);
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

      outputfile << nodeSize << std::endl;

      while (nodeIter != nodeIterEnd)
      {
        Point p;
        mesh->get_center(p, *nodeIter);
        outputfile << p.x() << " " << p.y() << " " << p.z() << std::endl;
        ++nodeIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write to file: " + pts_fn);
      return false;
    }
    outputfile.close();
  }

  // Elements file
  {
    std::ofstream outputfile;
    outputfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    std::string elems_fn(filename);
    std::string::size_type pos = elems_fn.find_last_of(".");
    std::string base = elems_fn.substr(0, pos);
    std::string ext = elems_fn.substr(pos);
    const char* fileExt = ".elem";

    if (pos == std::string::npos)
    {
      elems_fn += fileExt;
    }
    else if (ext != fileExt)
    {
      elems_fn = base + fileExt;
    }

    try
    {
      outputfile.open(elems_fn.c_str());

      VMesh::Cell::iterator cellIter;
      VMesh::Cell::iterator cellIterEnd;
      VMesh::Cell::size_type cellSize;

      mesh->begin(cellIter);
      mesh->end(cellIterEnd);
      mesh->size(cellSize);



#if DEBUG
      std::cerr << "Number of tets = " << cellSize << std::endl;
#endif

      outputfile << cellSize << std::endl;

      if (fi.is_tetvolmesh())
      {
        VMesh::Node::array_type cellNodes(4);

        double scalaroutput;

        while (cellIter != cellIterEnd) {

          field->get_value(scalaroutput, *cellIter);
          mesh->get_nodes(cellNodes, *cellIter);

          outputfile << "Tt" << " " << cellNodes[0] << " " << cellNodes[1] << " " << cellNodes[2] << " " << cellNodes[3] << " " << scalaroutput << std::endl;
          ++cellIter;
        }
      }
      else if (fi.is_trisurfmesh())
      {

        VMesh::Node::array_type faceNodes(3);

        double scalaroutput;

        while (cellIter != cellIterEnd) {

          field->get_value(scalaroutput, *cellIter);
          mesh->get_nodes(faceNodes, *cellIter);

          outputfile << "Tr" << " " << faceNodes[0] << " " << faceNodes[1] << " " << faceNodes[2] << " " << scalaroutput << std::endl;
          ++cellIter;
        }
      }
      else
      {

        if (pr) pr->error("Please convert to TetVol mesh ");
        return false;
      }

    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write to file: " + elems_fn);
      return false;
    }
    outputfile.close();
  }

  // Isotropic Fiber file
  {
    std::ofstream outputfile;
    outputfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    std::string lon_fn(filename);
    std::string::size_type pos = lon_fn.find_last_of(".");
    std::string base = lon_fn.substr(0, pos);
    std::string ext = lon_fn.substr(pos);
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
#endif

      outputfile << 1 << std::endl;

      std::cout << "The generated *.lon file assumes a bath in the smallest mask layer and assigns a fiber direction of [0 0 1] to all other data layers. If a different fiber file is required, please use the CarpFiber exporter/importer." << std::endl;

      if (fi.is_tetvolmesh())
      {

        VMesh::Node::array_type cellNodes(4);

        std::vector<double> region;
        double scalaroutput;
        double min_region;

        while (cellIter != cellIterEnd)
        {
          field->get_value(scalaroutput, *cellIter);
          region.push_back(scalaroutput);

          if (cellIter == 1)
          {
            min_region = scalaroutput;
          }
          else if (scalaroutput < min_region)
          {
            min_region = scalaroutput;
          }
          ++cellIter;
        }

        VMesh::Cell::iterator cellIter;

        while (cellIter != cellIterEnd)
        {
          if (region[*cellIter] != min_region)
          {
            outputfile << 0 << " " << 0 << " " << 1 << std::endl;
          }
          else
          {
            outputfile << 0 << " " << 0 << " " << 0 << std::endl;
          }

          ++cellIter;
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

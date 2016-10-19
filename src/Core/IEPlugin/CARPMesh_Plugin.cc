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


using namespace std;
using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

FieldHandle SCIRun::CARPMesh_reader(LoggerHandle pr, const char *filename)
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
    outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    std::string elems_fn(filename);
    std::string::size_type pos = elems_fn.find_last_of(".");
    std::string base = elems_fn.substr(0, pos);
    std::string ext  = elems_fn.substr(pos);
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
      
      	field->get_value(scalaroutput,*cellIter);
      	mesh->get_nodes(cellNodes, *cellIter);
      	
        outputfile << "Tt" << " " << cellNodes[0] << " " << cellNodes[1] << " " << cellNodes[2] << " " << cellNodes[3] << " " << scalaroutput << std::endl;
        ++cellIter;
      		}
      	}
	
      	else 
		{
      if (pr) pr->error("Please convert to Tetvol ");
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
#endif

	outputfile << 1 << std::endl;
	
	cout << "The generated *.lon file assumes a bath in the smallest mask layer and assigns a fiber direction of [0 0 1] to all other data layers. If a different fiber file is required, please use the CarpFiber exporter/importer." << endl;
	
	 if (fi.is_tetvolmesh())
  		{
	
	    VMesh::Node::array_type cellNodes(4);
	    
	    vector<double> region;
	    double scalaroutput;
	    double min_region;
	
      while (cellIter != cellIterEnd) {
      
      	field->get_value(scalaroutput,*cellIter);
      	region.push_back(scalaroutput);
      	
      	if (cellIter == 1){
      	min_region = scalaroutput;
      	}
      	else if (scalaroutput < min_region){
      	min_region=scalaroutput;
      	}
      	++cellIter;
      	}
      	
      	VMesh::Cell::iterator cellIter;
      
       while (cellIter != cellIterEnd){
      
    
      	if (region[*cellIter] != min_region) {
      	outputfile << 0 << " " << 0 << " " << 1 << std::endl;
      	}
      	else {
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


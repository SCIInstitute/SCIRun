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
#include <Core/Util/Endian.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Util/StringUtil.h>

#include <sci_debug.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <limits.h>

namespace SCIRun {

FieldHandle PVWave_reader(ProgressReporter *pr, const char *filename);

bool PVWave_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);

FieldHandle PVWave_reader(ProgressReporter *pr, const char *filename)
{
	FieldHandle result = 0;

	std::string var_fn(filename);

	// Check whether the .elems or .tri file exists
	std::string::size_type pos = var_fn.find_last_of(".");
	if (pos == std::string::npos)
	{
		var_fn = var_fn + ".var";
		try
		{
			std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
			inputfile.open(var_fn.c_str());
		}

		catch (...)
		{
			if (pr) pr->error("Could not open file: "+var_fn);
			return (result);
		}
	}
	else
	{
		std::string base = var_fn.substr(0,pos);
		std::string ext  = var_fn.substr(pos);
		if ((ext != ".var" ))
		{
			try
			{
				std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
				var_fn = base + ".var";
				inputfile.open(var_fn.c_str());
			}
			catch (...)
			{
        if (pr) pr->error("Could not open file: "+base + ".var");
        return (result);
			}
		}
		else
		{
			try
			{
				std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
				inputfile.open(var_fn.c_str());
			}
			catch (...)
			{
				if (pr) pr->error("Could not open file: "+var_fn);
				return (result);
			}
		}
	}

  int dim =0;
  std::vector<int> dims;
  int type;
  int sample_freq;

  std::ifstream inputfile;
  inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );

  // Try to read header
  try
  {
    inputfile.open(var_fn.c_str());
    inputfile.read(reinterpret_cast<char *>(&dim),4);
    if (isBigEndian()) swapbytes(dim);
    dims.resize(dim);
    for (int i=0; i<dim; i++)
    {
      inputfile.read(reinterpret_cast<char *>(&dims[i]),4);
      if (isBigEndian()) swapbytes(dims[i]);
    }
    inputfile.read(reinterpret_cast<char *>(&type),4);
    if (isBigEndian()) swapbytes(type);
    inputfile.read(reinterpret_cast<char *>(&sample_freq),4);
    if (isBigEndian()) swapbytes(sample_freq);
  }
  catch(...)
  {
    if (pr) pr->error("Could not read header of file: "+var_fn);
    return (result);
  }

  // Check integrity

  int num_elems = 1;
  for (int i=0;i<dim;i++) num_elems *= dims[i];

  if (dim > 3 || dim < 1)
  {
    if (pr) pr->error("We cannot read a PV-Wave file with more dimensions than 3 or less than 1.");
    return (result);
  }

  if (type < 1 || type < 5)
  {
    if (pr) pr->error("The type of the PV-Wave variable is not supported by SCIRun");
    return (result);
  }

  FieldInformation fi(LATVOLMESH_E,CONSTANTDATA_E,DOUBLE_E);

  MeshHandle mesh;
  if (dim == 3)
  {
    mesh = CreateMesh(fi,static_cast<size_type>(dims[0]+1),
                         static_cast<size_type>(dims[1]+1),
                         static_cast<size_type>(dims[2]+1),
                         Point(0.0,0.0,0.0),
                         Point(1.0,1.0,1.0));
  }
  else if (dim == 2)
  {
    fi.make_imagemesh();
    mesh = CreateMesh(fi,static_cast<size_type>(dims[0]+1),
                         static_cast<size_type>(dims[1]+1),
                         Point(0.0,0.0,0.0),
                         Point(1.0,1.0,0.0));

  }
  if (dim == 1)
  {
    fi.make_scanlinemesh();
    mesh = CreateMesh(fi,static_cast<size_type>(dims[0]+1),
                         Point(0.0,0.0,0.0),
                         Point(1.0,0.0,0.0));
  }

  if (type == 1)
  {
    fi.make_char();
    result = CreateField(fi,mesh);
    char* ptr = reinterpret_cast<char*>(result->vfield()->get_values_pointer());
    inputfile.read(ptr,num_elems*1);
    // No byte swapping needed
  }
  else if (type == 2)
  {
    fi.make_short();
    result = CreateField(fi,mesh);
    char* ptr = reinterpret_cast<char*>(result->vfield()->get_values_pointer());
    inputfile.read(ptr,num_elems*2);
    if (isBigEndian())
    {
      short* dptr = reinterpret_cast<short*>(ptr);
      for (int j=0;j<num_elems;j++) swapbytes(dptr[j]);
    }
  }
  if (type == 3)
  {
    fi.make_int();
    result = CreateField(fi,mesh);
    char* ptr = reinterpret_cast<char*>(result->vfield()->get_values_pointer());
    inputfile.read(ptr,num_elems*4);
    if (isBigEndian())
    {
      int* dptr = reinterpret_cast<int*>(ptr);
      for (int j=0;j<num_elems;j++) swapbytes(dptr[j]);
    }
  }
  if (type == 4)
  {
    fi.make_float();
    result = CreateField(fi,mesh);
    char* ptr = reinterpret_cast<char*>(result->vfield()->get_values_pointer());
    inputfile.read(ptr,num_elems*4);
    if (isBigEndian())
    {
      float* dptr = reinterpret_cast<float*>(ptr);
      for (int j=0;j<num_elems;j++) swapbytes(dptr[j]);
    }

  }
  if (type == 5)
  {
    fi.make_double();
    result = CreateField(fi,mesh);
    char* ptr = reinterpret_cast<char*>(result->vfield()->get_values_pointer());
    inputfile.read(ptr,num_elems*8);
    if (isBigEndian())
    {
      double* dptr = reinterpret_cast<double*>(ptr);
      for (int j=0;j<num_elems;j++) swapbytes(dptr[j]);
    }

  }

  inputfile.close();
  return (result);
}




bool PVWave_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
{
  VMesh*  mesh = fh->vmesh();
  VField* field = fh->vfield();

  if ((!mesh->is_latvolmesh()) && (!mesh->is_imagemesh()) && (!mesh->is_scanlinemesh()))
  {
    if (pr) pr->error("Mesh is not a regular structured mesh, cannot convert mesh to PV-wave fileformat");
    return (false);
  }

  if (field->basis_order() != 0)
  {
    if (pr) pr->error("Data needs to be located on the elements.");
    return (false);
  }

  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
  std::string var_fn(filename);
  std::string::size_type pos = var_fn.find_last_of(".");
  std::string base = var_fn.substr(0, pos);
  std::string ext  = var_fn.substr(pos);

  if (pos == std::string::npos)
  {
    var_fn += ".var";
  }
  else if (ext != ".var")
  {
    var_fn = base + ".var";
  }

  try
  {
    outputfile.open(var_fn.c_str());
  }
  catch (...)
  {
    if (pr) pr->error("Could not open file: "+ var_fn);
    return (false);
  }


  if (mesh->is_latvolmesh())
  {
    VMesh::dimension_type edims;
    mesh->get_elem_dimensions(edims);

    int dim = 3;
    std::vector<int> dims(3);
    for (size_t j=0; j<3; j++) dims[j] = static_cast<int>(edims[j]);

    int type = 0;
    if (field->is_char() || field->is_unsigned_char()) type = 1;
    if (field->is_short() || field->is_unsigned_short()) type = 2;
    if (field->is_int() || field->is_unsigned_int()) type = 3;
    if (field->is_float()) type = 4;
    if (field->is_double()) type = 5;

    int num_elems = 1;
    size_t num_elems_big = 1;
    for (size_t j =0; j<dims.size();j++) num_elems_big *= dims[j];
    if (num_elems_big > INT_MAX)
    {
      if (pr) pr->error("PVWave does not support datasets larger than 2Gb");
      return (false);
    }
    num_elems = static_cast<int>(num_elems_big);

    int sample_freq = 1000;

    if (isBigEndian())
    {
      swapbytes(dim);
      for (size_t j =0; j<dims.size();j++) swapbytes(dims[j]);
      swapbytes(type);
      swapbytes(num_elems);
      swapbytes(sample_freq);
    }

    outputfile.write(reinterpret_cast<const char*>(&dim),4);
    for (size_t j =0; j<dims.size();j++)
    {
      outputfile.write(reinterpret_cast<const char*>(&dims[j]),4);
    }
    outputfile.write(reinterpret_cast<const char*>(&type),4);
    outputfile.write(reinterpret_cast<const char*>(&sample_freq),4);

    if (isBigEndian())
    {
      swapbytes(dim);
      for (size_t j =0; j<dims.size();j++) swapbytes(dims[j]);
      swapbytes(type);
      swapbytes(num_elems);
    }

    if (field->is_char() || field->is_unsigned_char())
    {
      std::vector<char> data;
      field->get_values(data);
      char* ptr = reinterpret_cast<char *>(&data[0]);
      outputfile.write(ptr,num_elems);
    }
    else if (field->is_short() || field->is_unsigned_short())
    {
      std::vector<short> data;
      field->get_values(data);
      if (isBigEndian()) for (int j=0; j< num_elems; j++) swapbytes(data[j]);
      char* ptr = reinterpret_cast<char *>(&data[0]);
      outputfile.write(ptr,num_elems*2);
    }
    else if (field->is_int() || field->is_unsigned_int())
    {
      std::vector<int> data;
      field->get_values(data);
      if (isBigEndian()) for (int j=0; j< num_elems; j++) swapbytes(data[j]);
      char* ptr = reinterpret_cast<char *>(&data[0]);
      outputfile.write(ptr,num_elems*4);
    }
    else if (field->is_float())
    {
      std::vector<float> data;
      field->get_values(data);
      if (isBigEndian()) for (int j=0; j< num_elems; j++) swapbytes(data[j]);
      char* ptr = reinterpret_cast<char *>(&data[0]);
      outputfile.write(ptr,num_elems*4);
    }
    else if (field->is_double())
    {
      std::vector<double> data;
      field->get_values(data);
      if (isBigEndian()) for (int j=0; j< num_elems; j++) swapbytes(data[j]);
      char* ptr = reinterpret_cast<char *>(&data[0]);
      outputfile.write(ptr,num_elems*8);
    }
  }


  if (mesh->is_imagemesh())
  {
    VMesh::dimension_type edims;
    mesh->get_elem_dimensions(edims);

    int dim = 3;
    std::vector<int> dims(3);
    for (size_t j=0; j<2; j++) dims[j] = static_cast<int>(edims[j]);
    dims[2] = 1;

    int type = 0;
    if (field->is_char() || field->is_unsigned_char()) type = 1;
    if (field->is_short() || field->is_unsigned_short()) type = 2;
    if (field->is_int() || field->is_unsigned_int()) type = 3;
    if (field->is_float()) type = 4;
    if (field->is_double()) type = 5;

    int num_elems = 1;
    size_t num_elems_big = 1;
    for (size_t j =0; j<dims.size();j++) num_elems_big *= dims[j];
    if (num_elems_big > INT_MAX)
    {
      if (pr) pr->error("PVWave does not support datasets larger than 2Gb");
      return (false);
    }
    num_elems = static_cast<int>(num_elems_big);

    if (isBigEndian())
    {
      swapbytes(dim);
      for (size_t j =0; j<dims.size();j++) swapbytes(dims[j]);
      swapbytes(type);
      swapbytes(num_elems);
    }

    outputfile.write(reinterpret_cast<const char*>(&dim),4);
    for (size_t j =0; j<dims.size();j++)
    {
      outputfile.write(reinterpret_cast<const char*>(&dims[j]),4);
    }
    outputfile.write(reinterpret_cast<const char*>(&type),4);
    outputfile.write(reinterpret_cast<const char*>(&num_elems),4);

    if (isBigEndian())
    {
      swapbytes(dim);
      for (size_t j =0; j<dims.size();j++) swapbytes(dims[j]);
      swapbytes(type);
      swapbytes(num_elems);
    }

    if (field->is_char() || field->is_unsigned_char())
    {
      std::vector<char> data;
      field->get_values(data);
      char* ptr = reinterpret_cast<char *>(&data[0]);
      outputfile.write(ptr,num_elems);
    }
    else if (field->is_short() || field->is_unsigned_short())
    {
      std::vector<short> data;
      field->get_values(data);
      if (isBigEndian()) for (int j=0; j< num_elems; j++) swapbytes(data[j]);
      char* ptr = reinterpret_cast<char *>(&data[0]);
      outputfile.write(ptr,num_elems*2);
    }
    else if (field->is_int() || field->is_unsigned_int())
    {
      std::vector<int> data;
      field->get_values(data);
      if (isBigEndian()) for (int j=0; j< num_elems; j++) swapbytes(data[j]);
      char* ptr = reinterpret_cast<char *>(&data[0]);
      outputfile.write(ptr,num_elems*4);
    }
    else if (field->is_float())
    {
      std::vector<float> data;
      field->get_values(data);
      if (isBigEndian()) for (int j=0; j< num_elems; j++) swapbytes(data[j]);
      char* ptr = reinterpret_cast<char *>(&data[0]);
      outputfile.write(ptr,num_elems*4);
    }
    else if (field->is_double())
    {
      std::vector<double> data;
      field->get_values(data);
      if (isBigEndian()) for (int j=0; j< num_elems; j++) swapbytes(data[j]);
      char* ptr = reinterpret_cast<char *>(&data[0]);
      outputfile.write(ptr,num_elems*8);
    }
  }


  outputfile.close();
  return true;
}

static FieldIEPlugin PVWave_plugin("PVWave","{.var}", "", PVWave_reader, PVWave_writer);

} // end namespace

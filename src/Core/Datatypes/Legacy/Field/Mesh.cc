/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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



#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/Thread/Legacy/Mutex.h>
#include <sci_debug.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
// initialize the static member type_id
PersistentTypeID Mesh::type_id("Mesh", "PropertyManager", NULL);
#endif


namespace 
{
// A list to keep a record of all the different Field types that
// are supported through a virtual interface
Mutex *MeshTypeIDMutex = 0;
static std::map<std::string,MeshTypeID*>* MeshTypeIDTable = 0;
}

MeshTypeID::MeshTypeID(const std::string&type, MeshHandle (*mesh_maker)()) :
    type(type),
    mesh_maker(mesh_maker),
    latvol_maker(0),
    image_maker(0),
    scanline_maker(0),
    structhexvol_maker(0),
    structquadsurf_maker(0),
    structcurve_maker(0)
{
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  if (MeshTypeIDTable == 0)
  {
    MeshTypeIDTable = new std::map<std::string,MeshTypeID*>;
  }
  else
  {
    std::map<std::string,MeshTypeID*>::iterator dummy;
    
    dummy = MeshTypeIDTable->find(type);
    
    if (dummy != MeshTypeIDTable->end())
    {
      if ((*dummy).second->mesh_maker != mesh_maker)
      {
#if DEBUG
        std::cerr << "WARNING: duplicate mesh type exists: " << type << "\n";
#endif
        MeshTypeIDMutex->unlock();
        return;
      }
    }
  }
  
  (*MeshTypeIDTable)[type] = this;
  MeshTypeIDMutex->unlock();
}

MeshTypeID::MeshTypeID(const std::string&type,
		       MeshHandle (*mesh_maker)(),
		       MeshHandle (*latvol_maker)(size_type x,
						  size_type y,
						  size_type z,
						  const Point& min,
						  const Point& max) ) :
  type(type),
  mesh_maker(mesh_maker),
  latvol_maker(latvol_maker),
  image_maker(0),
  scanline_maker(0),
  structhexvol_maker(0),
  structquadsurf_maker(0),
  structcurve_maker(0)
{
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  if (MeshTypeIDTable == 0)
  {
    MeshTypeIDTable = new std::map<std::string,MeshTypeID*>;
  }
  else
  {
    std::map<std::string,MeshTypeID*>::iterator dummy;
    
    dummy = MeshTypeIDTable->find(type);
    
    if (dummy != MeshTypeIDTable->end())
    {
      if ((*dummy).second->mesh_maker != mesh_maker)
      {
#if DEBUG
        std::cerr << "WARNING: duplicate mesh type exists: " << type << "\n";
#endif
        MeshTypeIDMutex->unlock();
        return;
      }
    }
  }
  
  (*MeshTypeIDTable)[type] = this;
  MeshTypeIDMutex->unlock();
}

MeshTypeID::MeshTypeID(const std::string&type,
		       MeshHandle (*mesh_maker)(),
		       MeshHandle (*image_maker)(size_type x,
						 size_type y,
						 const Point& min,
						 const Point& max) ) :
  type(type),
  mesh_maker(mesh_maker),
  latvol_maker(0),
  image_maker(image_maker),
  scanline_maker(0),
  structhexvol_maker(0),
  structquadsurf_maker(0),
  structcurve_maker(0)
{
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  if (MeshTypeIDTable == 0)
  {
    MeshTypeIDTable = new std::map<std::string,MeshTypeID*>;
  }
  else
  {
    std::map<std::string,MeshTypeID*>::iterator dummy;
    
    dummy = MeshTypeIDTable->find(type);
    
    if (dummy != MeshTypeIDTable->end())
    {
      if ((*dummy).second->mesh_maker != mesh_maker)
      {
#if DEBUG
        std::cerr << "WARNING: duplicate mesh type exists: " << type << "\n";
#endif
        MeshTypeIDMutex->unlock();
        return;
      }
    }
  }
  
  (*MeshTypeIDTable)[type] = this;
  MeshTypeIDMutex->unlock();
}

MeshTypeID::MeshTypeID(const std::string&type,
		       MeshHandle (*mesh_maker)(),
		       MeshHandle (*scanline_maker)(size_type x,
						    const Point& min,
						    const Point& max) ) :
  type(type),
  mesh_maker(mesh_maker),
  latvol_maker(0),
  image_maker(0),
  scanline_maker(scanline_maker),
  structhexvol_maker(0),
  structquadsurf_maker(0),
  structcurve_maker(0)
{
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  if (MeshTypeIDTable == 0)
  {
    MeshTypeIDTable = new std::map<std::string,MeshTypeID*>;
  }
  else
  {
    std::map<std::string,MeshTypeID*>::iterator dummy;
    
    dummy = MeshTypeIDTable->find(type);
    
    if (dummy != MeshTypeIDTable->end())
    {
      if ((*dummy).second->mesh_maker != mesh_maker)
      {
#if DEBUG
        std::cerr << "WARNING: duplicate mesh type exists: " << type << "\n";
#endif
        MeshTypeIDMutex->unlock();
        return;
      }
    }
  }
  
  (*MeshTypeIDTable)[type] = this;
  MeshTypeIDMutex->unlock();
}


MeshTypeID::MeshTypeID(const std::string&type,
		       MeshHandle (*mesh_maker)(),
		       MeshHandle (*structhexvol_maker)(size_type x,
							size_type y,
							size_type z) ) :
  type(type),
  mesh_maker(mesh_maker),
  latvol_maker(0),
  image_maker(0),
  scanline_maker(0),
  structhexvol_maker(structhexvol_maker),
  structquadsurf_maker(0),
  structcurve_maker(0)
{
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  if (MeshTypeIDTable == 0)
  {
    MeshTypeIDTable = new std::map<std::string,MeshTypeID*>;
  }
  else
  {
    std::map<std::string,MeshTypeID*>::iterator dummy;
    
    dummy = MeshTypeIDTable->find(type);
    
    if (dummy != MeshTypeIDTable->end())
    {
      if ((*dummy).second->mesh_maker != mesh_maker)
      {
#if DEBUG
        std::cerr << "WARNING: duplicate mesh type exists: " << type << "\n";
#endif
        MeshTypeIDMutex->unlock();
        return;
      }
    }
  }
  
  (*MeshTypeIDTable)[type] = this;
  MeshTypeIDMutex->unlock();
}

MeshTypeID::MeshTypeID(const std::string&type,
		       MeshHandle (*mesh_maker)(),
		       MeshHandle (*structquadsurf_maker)(size_type x,
							  size_type y) ) :
  type(type),
  mesh_maker(mesh_maker),
  latvol_maker(0),
  image_maker(0),
  scanline_maker(0),
  structhexvol_maker(0),
  structquadsurf_maker(structquadsurf_maker),
  structcurve_maker(0)
{
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  if (MeshTypeIDTable == 0)
  {
    MeshTypeIDTable = new std::map<std::string,MeshTypeID*>;
  }
  else
  {
    std::map<std::string,MeshTypeID*>::iterator dummy;
    
    dummy = MeshTypeIDTable->find(type);
    
    if (dummy != MeshTypeIDTable->end())
    {
      if ((*dummy).second->mesh_maker != mesh_maker)
      {
#if DEBUG
        std::cerr << "WARNING: duplicate mesh type exists: " << type << "\n";
#endif
        MeshTypeIDMutex->unlock();
        return;
      }
    }
  }
  
  (*MeshTypeIDTable)[type] = this;
  MeshTypeIDMutex->unlock();
}


MeshTypeID::MeshTypeID(const std::string& type,
		       MeshHandle (*mesh_maker)(),
		       MeshHandle (*structcurve_maker)(size_type x) ) :
  type(type),
  mesh_maker(mesh_maker),
  latvol_maker(0),
  image_maker(0),
  scanline_maker(0),
  structhexvol_maker(0),
  structquadsurf_maker(0),
  structcurve_maker(structcurve_maker)
{
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  if (MeshTypeIDTable == 0)
  {
    MeshTypeIDTable = new std::map<std::string,MeshTypeID*>;
  }
  else
  {
    std::map<std::string,MeshTypeID*>::iterator dummy;
    
    dummy = MeshTypeIDTable->find(type);
    
    if (dummy != MeshTypeIDTable->end())
    {
      if ((*dummy).second->mesh_maker != mesh_maker)
      {
#if DEBUG
        std::cerr << "WARNING: duplicate mesh type exists: " << type << "\n";
#endif
        MeshTypeIDMutex->unlock();
        return;
      }
    }
  }
  
  (*MeshTypeIDTable)[type] = this;
  MeshTypeIDMutex->unlock();
}


Mesh::Mesh()
{
  DEBUG_CONSTRUCTOR("Mesh")  
}

Mesh::~Mesh() 
{
  DEBUG_DESTRUCTOR("Mesh")  
}

int
Mesh::basis_order()
{
  return (-1);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
const int MESHBASE_VERSION = 2;

void 
Mesh::io(Piostream& stream)
{
  if (stream.reading() && stream.peek_class() == "MeshBase")
  {
    stream.begin_class("MeshBase", 1);
  }
  else
  {
    stream.begin_class("Mesh", MESHBASE_VERSION);
  }
  PropertyManager::io(stream);
  stream.end_class();
}

const std::string 
Mesh::type_name(int n)
{
  ASSERT(n >= -1 && n <= 0);
  static const std::string name = "Mesh";
  return name;
}
#endif

//! Return the transformation that takes a 0-1 space bounding box to
//! the current bounding box of this mesh.
//void Mesh::get_canonical_transform(Transform &t) 
//{
//  t.load_identity();
//  BBox bbox = get_bounding_box();
//  t.pre_scale(bbox.diagonal());
//  t.pre_translate(Vector(bbox.min()));
//}

//! This function should be overloaded with the actual function that
//! retrieves the virtual interface. This function is thread safe, but
//! is not const as it deals with handles which will alter ref counts.
VMesh* 
Mesh::vmesh()
{
  return (0);
} 



MeshHandle
CreateMesh(const std::string& type)
{
  MeshHandle handle;
  
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  std::map<std::string,MeshTypeID*>::iterator it;
  it = MeshTypeIDTable->find(type);
  if (it != MeshTypeIDTable->end()) 
  {
    if ((*it).second->mesh_maker != 0)
    {
      handle = (*it).second->mesh_maker();
    }
  }
  MeshTypeIDMutex->unlock();
  return (handle);
}

MeshHandle
CreateMesh(const std::string& type,
	   Mesh::size_type x, Mesh::size_type y, Mesh::size_type z,
	   const Point& min, const Point& max)
{
  MeshHandle handle;
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  std::map<std::string,MeshTypeID*>::iterator it;
  it = MeshTypeIDTable->find(type);
  if (it != MeshTypeIDTable->end()) 
  {
    if ((*it).second->latvol_maker != 0)
    {
      handle = (*it).second->latvol_maker(x,y,z,min,max);
    }
  }
  MeshTypeIDMutex->unlock();
  return (handle);
}

MeshHandle
CreateMesh(const std::string& type,
	   Mesh::size_type x, Mesh::size_type y,
	   const Point& min, const Point& max)
{
  MeshHandle handle;
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  std::map<std::string,MeshTypeID*>::iterator it;
  it = MeshTypeIDTable->find(type);
  if (it != MeshTypeIDTable->end()) 
  {
    if ((*it).second->image_maker != 0)
    {
      handle = (*it).second->image_maker(x,y,min,max);
    }
  }
  MeshTypeIDMutex->unlock();
  return (handle);
}

MeshHandle
CreateMesh(const std::string& type, Mesh::size_type x,
	   const Point& min, const Point& max)
{
  MeshHandle handle;
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  std::map<std::string,MeshTypeID*>::iterator it;
  it = MeshTypeIDTable->find(type);
  if (it != MeshTypeIDTable->end()) 
  {
    if ((*it).second->scanline_maker != 0)
    {
      handle = (*it).second->scanline_maker(x,min,max);
    }
  }
  MeshTypeIDMutex->unlock();
  return (handle);
}

MeshHandle
CreateMesh(const std::string& type,
	   Mesh::size_type x, Mesh::size_type y, Mesh::size_type z)
{
  MeshHandle handle;
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  std::map<std::string,MeshTypeID*>::iterator it;
  it = MeshTypeIDTable->find(type);
  if (it != MeshTypeIDTable->end()) 
  {
    if ((*it).second->structhexvol_maker != 0)
    {
      handle = (*it).second->structhexvol_maker(x,y,z);
    }
    if ((*it).second->latvol_maker != 0)
    {
      handle = (*it).second->latvol_maker(x,y,z,Point(0,0,0),Point(1,1,1));
    }    
  }
  MeshTypeIDMutex->unlock();
  return (handle);
}

MeshHandle
CreateMesh(const std::string& type, Mesh::size_type x, Mesh::size_type y)
{
  MeshHandle handle;
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  std::map<std::string,MeshTypeID*>::iterator it;
  it = MeshTypeIDTable->find(type);
  if (it != MeshTypeIDTable->end()) 
  {
    if ((*it).second->structquadsurf_maker != 0)
    {
      handle = (*it).second->structquadsurf_maker(x,y);
    }
    if ((*it).second->image_maker != 0)
    {
      handle = (*it).second->image_maker(x,y,Point(0,0,0),Point(1,1,0));
    }
  }
  MeshTypeIDMutex->unlock();
  return (handle);
}

MeshHandle
CreateMesh(const std::string& type, Mesh::size_type x)
{
  MeshHandle handle;
  if (MeshTypeIDMutex == NULL)
  {
    MeshTypeIDMutex = new Mutex("Mesh Type ID Table Lock");
  }
  MeshTypeIDMutex->lock();
  std::map<std::string,MeshTypeID*>::iterator it;
  it = MeshTypeIDTable->find(type);
  if (it != MeshTypeIDTable->end()) 
  {
    if ((*it).second->structcurve_maker != 0)
    {
      handle = (*it).second->structcurve_maker(x);  
    }
    if ((*it).second->scanline_maker != 0)
    {
      handle = (*it).second->scanline_maker(x,Point(0,0,0),Point(1,0,0));  
    }
  }
  MeshTypeIDMutex->unlock();
  return (handle);
}

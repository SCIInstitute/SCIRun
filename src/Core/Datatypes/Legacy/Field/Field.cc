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



#include <Core/Datatypes/Legacy/Field/Field.h>

#include <Core/Utils/Legacy/ProgressReporter.h>
#include <Core/Thread/Mutex.h>
#include <sci_debug.h>

#include <map>

using namespace SCIRun;
using namespace SCIRun::Core::Thread;

Field::Field()
{
  DEBUG_CONSTRUCTOR("Field")  
}

Field::~Field()
{
  DEBUG_DESTRUCTOR("Field")  
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
const int FIELD_VERSION = 3;

void 
Field::io(Piostream& stream)
{
  int version = stream.begin_class("Field", FIELD_VERSION);
  if (version < 2) 
  {
    // The following was FIELD_VERSION 1 data_at ordering
    //     enum data_location{
    //       NODE,
    //       EDGE,
    //       FACE,
    //       CELL,
    //       NONE
    //     };

    unsigned int tmp;
    int order = 999;
    Pio(stream, tmp);
    if (tmp == 0) 
    {
      // data_at_ was NODE
      order = 1;
    } 
    else if (tmp == 4) 
    {
      // data_at_ was NONE
      order = -1;
    } 
    else 
    {
      // data_at_ was somewhere else
      order = 0;
    }
    
    if (order != basis_order() && ((!(vmesh()->is_pointcloudmesh())) || order == -1)) 
    {
      // signal error in the stream and return;
      stream.flag_error();
      return;
    }
  } 
  else if (version < 3) 
  {
    int order;
    Pio(stream, order);
    if (order != basis_order() && ((!(vmesh()->is_pointcloudmesh())) || order == -1)) 
    {
      // signal error in the stream and return;
      stream.flag_error();
      return;
    }
  }

  PropertyManager::io(stream);
  stream.end_class();
}

// initialize the static member type_id
PersistentTypeID Field::type_id("Field", "PropertyManager", 0);
#endif

// A list to keep a record of all the different Field types that
// are supported through a virtual interface
Mutex *FieldTypeIDMutex = 0;
static std::map<std::string,FieldTypeID*>* FieldTypeIDTable = 0;

FieldTypeID::FieldTypeID(const std::string&type,
                         FieldHandle (*field_maker)(),
                         FieldHandle (*field_maker_mesh)(MeshHandle)) :
    type(type),
    field_maker(field_maker),
    field_maker_mesh(field_maker_mesh)
{
  if (FieldTypeIDMutex == 0)
  {
    FieldTypeIDMutex = new Mutex("Field Type ID Table Lock");
  }
  FieldTypeIDMutex->lock();
  if (FieldTypeIDTable == 0)
  {
    FieldTypeIDTable = new std::map<std::string,FieldTypeID*>;
  }
  else
  {
    std::map<std::string,FieldTypeID*>::iterator dummy;
    
    dummy = FieldTypeIDTable->find(type);
    
    if (dummy != FieldTypeIDTable->end())
    {
      if (((*dummy).second->field_maker != field_maker) ||
          ((*dummy).second->field_maker_mesh != field_maker_mesh))
      {
#if DEBUG
        std::cerr << "WARNING: duplicate field type exists: " << type << "\n";
#endif
        FieldTypeIDMutex->unlock();
        return;
      }
    }
  }
  
  (*FieldTypeIDTable)[type] = this;
  FieldTypeIDMutex->unlock();
}


FieldHandle
CreateField(const std::string& type, MeshHandle mesh)
{
  FieldHandle handle;
  if (FieldTypeIDMutex == 0)
  {
    FieldTypeIDMutex = new Mutex("Field Type ID Table Lock");
  }
  FieldTypeIDMutex->lock();
  std::map<std::string,FieldTypeID*>::iterator it;
  it = FieldTypeIDTable->find(type);
  if (it != FieldTypeIDTable->end()) 
  {
    handle = (*it).second->field_maker_mesh(mesh);
  }
  else
  {
    std::cout << "Cannot find "<<type<<" in database\n";
  }
  FieldTypeIDMutex->unlock();
  return (handle);
}

FieldHandle
CreateField(const std::string& type)
{
  FieldHandle handle;
  if (FieldTypeIDMutex == 0)
  {
    FieldTypeIDMutex = new Mutex("Field Type ID Table Lock");
  }
  FieldTypeIDMutex->lock();
  std::map<std::string,FieldTypeID*>::iterator it;
  it = FieldTypeIDTable->find(type);
  if (it != FieldTypeIDTable->end()) 
  {
    handle = (*it).second->field_maker();
  }
  FieldTypeIDMutex->unlock();
  return (handle);
}

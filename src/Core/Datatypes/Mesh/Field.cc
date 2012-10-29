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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#include <boost/thread/mutex.hpp>
#include <map>
#include <Core/Datatypes/Mesh/Field.h>

using namespace SCIRun::Core::Datatypes;

Field::Field()
{
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  DEBUG_CONSTRUCTOR("Field")  
#endif
}

Field::~Field()
{
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  DEBUG_DESTRUCTOR("Field")  
#endif
}

const int FIELD_VERSION = 3;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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

PersistentTypeID Field::type_id("Field", "PropertyManager", 0);

#endif

//TODO DAN: REFACTORING NEEDED: LEVEL HIGH
// A list to keep a record of all the different Field types that
// are supported through a virtual interface
static boost::mutex *FieldTypeIDMutex = 0;
static std::map<std::string,FieldTypeID*>* FieldTypeIDTable = 0;

FieldTypeID::FieldTypeID(const std::string& type, FieldMaker fm, FieldMakerFromMesh fmm) :
    type(type),
    field_maker(field_maker),
    field_maker_mesh(field_maker_mesh)
{
  if (!FieldTypeIDMutex)
  {
    FieldTypeIDMutex = new boost::mutex(/*"Field Type ID Table Lock"*/);
  }

  boost::mutex::scoped_lock lock(*FieldTypeIDMutex);
  if (!FieldTypeIDTable)
  {
    FieldTypeIDTable = new std::map<std::string,FieldTypeID*>;
  }
  else
  {
    auto it = FieldTypeIDTable->find(type);
    
    if (it != FieldTypeIDTable->end())
    {
      //TODO: use real logger here
      std::cout << "WARNING: duplicate field type exists: " << type << std::endl;
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER //can't compare boost::function--switch back to native fn ptr?
      if ((it->second->field_maker != field_maker) ||
          (it->second->field_maker_mesh != field_maker_mesh))
      {
#if DEBUG
        //TODO: use real logger here
        std::cerr << "WARNING: duplicate field type exists: " << type << "\n";
#endif
        return;

      }
#endif
    }
  }
  
  (*FieldTypeIDTable)[type] = this;
}


FieldHandle
CreateField(const std::string& type, MeshHandle mesh)
{
  if (!FieldTypeIDMutex)
  {
    FieldTypeIDMutex = new boost::mutex(/*"Field Type ID Table Lock"*/);
  }
  boost::mutex::scoped_lock lock(*FieldTypeIDMutex);
  auto it = FieldTypeIDTable->find(type);
  if (it != FieldTypeIDTable->end()) 
  {
    return it->second->field_maker_mesh(mesh);
  }
  else
  {
    //TODO: use real logger here
    std::cout << "Cannot find "<<type<<" in database\n";
  }
  return FieldHandle();
}

FieldHandle
CreateField(const std::string& type)
{
  if (!FieldTypeIDMutex)
  {
    FieldTypeIDMutex = new boost::mutex(/*"Field Type ID Table Lock"*/);
  }
  boost::mutex::scoped_lock lock(*FieldTypeIDMutex);

  auto it = FieldTypeIDTable->find(type);
  if (it != FieldTypeIDTable->end()) 
  {
    return it->second->field_maker();
  }
  return FieldHandle();
}

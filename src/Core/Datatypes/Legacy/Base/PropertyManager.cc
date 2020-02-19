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


///
///@file  PropertyManager.cc
///
///@author
///       Yarden Livnat
///       Department of Computer Science
///       University of Utah
///
///@date  March 2001
///
///@brief Manage properties of persistent objects.
///

#include <Core/Datatypes/Legacy/Base/PropertyManager.h>
#include <Core/Utils/Legacy/MemoryUtil.h>

using namespace SCIRun::Core::Thread;

namespace SCIRun {

/// @todo: should be using Guards here...
PersistentTypeID
PropertyBase::type_id("PropertyBase", "Persistent", maker);

std::string PropertyBase::dynamic_type_name() const { return type_id.type; }

Persistent* PropertyBase::maker()
{
  return new PropertyBase(false);
}


Persistent* make_PropertyManager()
{
  return new PropertyManager;
}

PersistentTypeID PropertyManager::type_id("PropertyManager",
					  "Persistent",
					  make_PropertyManager);

std::string PropertyManager::dynamic_type_name() const { return type_id.type; }


PropertyManager::PropertyManager() :
  frozen_(false), lock("PropertyManager")
{
}


// PropertyManagers are created thawed.  Only non transient data is copied.
//
/// @todo: this and copy properties share essentially common code.
// Add a helper function to manage duplicated code.
PropertyManager::PropertyManager(const PropertyManager &copy) :
  Persistent(copy),
  frozen_(false),
  lock("PropertyManager")
{
  Guard this_guard(lock.get());
  Guard copy_guard(const_cast<PropertyManager&>(copy).lock.get());

  map_type::const_iterator pi = copy.properties_.begin();
  while (pi != copy.properties_.end())
  {
    if (! pi->second->transient())
    {
      properties_[pi->first].reset(pi->second->clone());
    }
    ++pi;
  }
}


PropertyManager &
PropertyManager::operator=(const PropertyManager &src)
{
  copy_properties(&src);
  return *this;
}

/// @todo: currently, field types are passing in pointers
// usually obtained from LockingHandle::get_rep().
// We should either change this to take a reference, or
// a smart pointer
void
PropertyManager::copy_properties(const PropertyManager* src)
{
  Guard this_guard(lock.get());

  clear_transient();
  frozen_ = false;

  {
    Guard src_guard(const_cast<PropertyManager*>(src)->lock.get());
    map_type::const_iterator pi = src->properties_.begin();
    while (pi != src->properties_.end())
    {
      if (! pi->second->transient())
      {
        properties_[pi->first].reset(pi->second->clone());
      }
      ++pi;
    }
  }

  frozen_ = true;
}


bool
PropertyManager::operator==(const PropertyManager &pm)
{
  bool result = true;

  Guard g(lock.get());

  if (nproperties() != pm.nproperties())
  {
    result = false;
  }

  Guard src_guard(const_cast<PropertyManager&>(pm).lock.get());
  map_type::const_iterator pi = pm.properties_.begin();
  while (result && pi != pm.properties_.end())
  {
    map_type::iterator loc = properties_.find(pi->first);

    if (loc == properties_.end() )
    {
      result = false;
    }
    else if( *(pi->second) != *(loc->second) )
    {
      result = false;
    }

    ++pi;
  }

  return result;
}


bool
PropertyManager::operator!=(const PropertyManager &pm)
{
  return ! (*this == pm);
}


PropertyManager::~PropertyManager()
{
  Guard g(lock.get());
  properties_.clear();
}


void
PropertyManager::thaw()
{
  // Assert that detach has been called on any handles to this PropertyManager.

 /// @todo: since we're using Boost shared pointers, ref_cnt is no longer u
 ///sed.
  // Do we need to verify that all handles are detached?
  //ASSERT(ref_cnt <= 1);

  // Clean up properties.
  Guard g(lock.get());

  clear_transient();
  frozen_ = false;
}


void
PropertyManager::freeze()
{
  Guard g(lock.get());

  frozen_ = true;
}


bool
PropertyManager::is_property(const std::string &name)
{
  Guard g(lock.get());

  bool ans = false;
  map_type::iterator loc = properties_.find(name);
  if (loc != properties_.end())
    ans = true;

  return ans;
}


std::string
PropertyManager::get_property_name(size_t index)
{
  if (index < nproperties())
  {
    Guard g(lock.get());

    map_type::const_iterator pi = properties_.begin();

    for(size_t i=0; i<index; i++ )
      ++pi;

    std::string result = pi->first;

    return result;
  }
  else
  {
    return std::string();
  }
}


void
PropertyManager::remove_property( const std::string &name )
{
  Guard g(lock.get());

  map_type::iterator loc = properties_.find(name);
  if (loc != properties_.end())
  {
    properties_.erase(name);
  }
}


void
PropertyManager::clear_transient()
{
  bool found;
  do {
    found = false;
    map_type::iterator iter = properties_.begin();
    while (iter != properties_.end())
    {
      auto p = *iter;
      if (p.second->transient())
      {
        properties_.erase(iter);
        found = true;
        break;
      }
      ++iter;
    }
  } while (found);
}


#define PROPERTYMANAGER_VERSION 2

void
PropertyManager::io(Piostream &stream)
{
  bool bc = stream.backwards_compat_id();
  stream.set_backwards_compat_id(false);


  const int version = stream.begin_class("PropertyManager", PROPERTYMANAGER_VERSION);
  if ( stream.writing() )
  {
    Guard g(lock.get());
    PropertyManagerSize nprop = nproperties();
    Pio(stream, nprop);
    for (auto& p : properties_)
    {
      std::string name = p.first;
      Pio(stream, name);
      PersistentHandle x = p.second;
      stream.io(x, PropertyBase::type_id);
    }
  }
  else
  {
    PropertyManagerSize size;
    Pio( stream, size );
    Guard g(lock.get());

    for (unsigned int i=0; i<size; i++ )
    {
      std::string name;
      Pio(stream, name );
      PersistentHandle p;
      stream.io( p, PropertyBase::type_id );
      properties_[name] = boost::static_pointer_cast<PropertyBase>(p);
      if (version < 2 && name == "minmax")
      {
        properties_[name]->set_transient(true);
      }
    }
  }
  stream.end_class();
  stream.set_backwards_compat_id(bc);
}


} // namespace SCIRun

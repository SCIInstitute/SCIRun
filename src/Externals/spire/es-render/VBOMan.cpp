
#include <set>
#include <stdexcept>
#include <string>

#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>

#include "VBOMan.hpp"
#include "comp/StaticVBOMan.hpp"
#include "comp/VBO.hpp"

namespace es = CPM_ES_NS;
namespace shaders = CPM_GL_SHADERS_NS;

namespace ren {

VBOMan::VBOMan()
{
}

VBOMan::~VBOMan()
{
  for (auto it = mVBOData.begin(); it != mVBOData.end(); ++it)
  {
    GLuint idToErase = it->first;
    GL(glDeleteBuffers(1, &idToErase));
  }
  mVBOData.clear();
}

void VBOMan::addVBOAttributes(GLuint glid, const std::vector<std::tuple<std::string, size_t, bool>>& attribs,
                              const std::string& assetName)
{
  // GCC 4.7.2 doesn't support emplace on maps.
  auto res = mVBOData.insert(std::make_pair(glid, VBOData(attribs, assetName.c_str())));

  if (std::get<1>(res) == false)
  {
    // Check the iterator to ensure the attributes are the same.
    auto mapIt = std::get<0>(res);
    if (mapIt->second.attributes.size() != attribs.size())
    {
      std::cerr << "Attributes for given glid already exist. But attributes are different.";
      throw std::runtime_error("Differing attributes");
      return;
    }

    // Since the sizes are the same, ensure that all of the attributes are
    // ordered the same, and have the same values.
    auto it = attribs.begin();
    for (const shaders::ShaderAttribute& attrib : mapIt->second.attributes)
    {
      if (   attrib.nameInCode != std::get<0>(*it)
          || attrib.sizeBytes != std::get<1>(*it))
      {
        std::cerr << "Attributes for given glid are different than pre-existing glid." << std::endl;
        throw std::runtime_error("Differing attributes");
        return;
      }
      ++it;
    }
  }
}

const std::vector<shaders::ShaderAttribute>& VBOMan::getVBOAttributes(GLuint glid) const
{
  auto it = mVBOData.find(glid);
  if (it != mVBOData.end())
  {
    return it->second.attributes;
  }
  else
  {
    std::cerr << "Unable to find attribute: " << glid << std::endl;
    throw std::runtime_error("Unable to find attribute.");
  }
}


GLuint VBOMan::hasVBO(const std::string& assetName) const
{
  /// \todo Strip off .geom, or add .geom?
  for (auto it = mVBOData.begin(); it != mVBOData.end(); ++it)
  {
    if (it->second.assetName == assetName)
    {
      return it->first;
    }
  }
  return 0;
}

GLuint VBOMan::addInMemoryVBO(void* vboData, size_t vboDataSize,
                              const std::vector<std::tuple<std::string, size_t, bool>>& attribs,
                              const std::string& assetName)
{
  GLuint glid;

  // Setup the VBO from the data given.
  GL(glGenBuffers(1, &glid));
  GL(glBindBuffer(GL_ARRAY_BUFFER, glid));
  GL(glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vboDataSize),
                  vboData, GL_STATIC_DRAW));

  addVBOAttributes(glid, attribs, assetName);

  return glid;
}

//------------------------------------------------------------------------------
// GARBAGE COLLECTION
//------------------------------------------------------------------------------

void VBOMan::runGCAgainstVaidIDs(const std::set<GLuint>& validKeys)
{
  // Every GLuint in validKeys should be in our map. If there is not, then
  // there is an error in the system, and it should be reported.
  // The reverse is not expected to be true, and is what we are attempting to
  // correct with this function.
  auto it = mVBOData.begin();
  for (const GLuint& id : validKeys)
  {
    // Find the key in the map, eliminating any keys that do not match the
    // current id along the way.
    while (it != mVBOData.end() && it->first < id)
    {
      //\cb std::cout << "VBO GC: " << it->second.assetName << std::endl;

      GLuint idToErase = it->first;
      mVBOData.erase(it++);
      GL(glDeleteBuffers(1, &idToErase));
    }

    if (it == mVBOData.end())
    {
      std::cerr << "runGCAgainstVaidIDs: terminating early, validKeys contains "
                << "elements not in VBO map." << std::endl;
      break;
    }

    // Check to see if the valid ids contain a component that is not in
    // mVBOData. If an object manages its own VBO, but still uses the VBO
    // component, this is not an error.
    if (it->first > id)
    {
      std::cerr << "runGCAgainstVaidIDs: validKeys contains elements not in the VBO map." << std::endl;
    }

    // Increment passed current validKey id.
    ++it;
  }

  while (it != mVBOData.end())
  {
    //\cb std::cout << "VBO GC: " << it->second.assetName << std::endl;

    GLuint idToErase = it->first;
    mVBOData.erase(it++);
    GL(glDeleteBuffers(1, &idToErase));
  }
}

class VBOGarbageCollector :
    public es::GenericSystem<false, VBO>
{
public:

  static const char* getName()    {return "ren:VBOGarbageCollector";}

  std::set<GLuint> mValidKeys;

  void preWalkComponents(es::ESCoreBase&) {mValidKeys.clear();}

  void postWalkComponents(es::ESCoreBase& core)
  {
    std::weak_ptr<ren::VBOMan> man = core.getStaticComponent<StaticVBOMan>()->instance_;
    if (std::shared_ptr<VBOMan> vboMan = man.lock()) {
          vboMan->runGCAgainstVaidIDs(mValidKeys);
          mValidKeys.clear();
    }
    else
    {
      std::cerr << "VBOMan: StaticVBOMan not found is given core. Failed to run VBO GC." << std::endl;
    }
  }

  void execute(es::ESCoreBase&, uint64_t /* entityID */, const VBO* vbo) override
  {
    mValidKeys.insert(vbo->glid);
  }
};

void VBOMan::registerSystems(CPM_ES_ACORN_NS::Acorn& core)
{
  // Register the garbage collector.
  core.registerSystem<VBOGarbageCollector>();
}

void VBOMan::runGCCycle(CPM_ES_NS::ESCoreBase& core)
{
  VBOGarbageCollector gc;
  gc.walkComponents(core);
}

const char* VBOMan::getGCName()
{
  return VBOGarbageCollector::getName();
}

} // namespace ren


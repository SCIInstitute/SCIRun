
#include <set>
#include <stdexcept>
#include <string>

#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>

#include "IBOMan.hpp"
#include "comp/StaticIBOMan.hpp"
#include "comp/IBO.hpp"

namespace es = CPM_ES_NS;
namespace shaders = CPM_GL_SHADERS_NS;

namespace ren {

IBOMan::IBOMan()
{
}

IBOMan::~IBOMan()
{
  for (auto it = mIBOData.begin(); it != mIBOData.end(); ++it)
  {
    GLuint idToErase = it->first;
    GL(glDeleteBuffers(1, &idToErase));
  }
  mIBOData.clear();
}

GLuint IBOMan::addInMemoryIBO(void* iboData, size_t iboDataSize, GLenum primMode,
                              GLenum primType, GLsizei numPrims,
                              const std::string& assetName)
{
  GLuint glid;

  GL(glGenBuffers(1, &glid));
  GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glid));
  GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(iboDataSize),
                  iboData, GL_STATIC_DRAW));

  mIBOData.insert(std::make_pair(glid, IBOData(assetName, primMode, primType, numPrims)));

  return glid;
}

void IBOMan::removeInMemoryIBO(GLuint glid)
{
  auto iter =  mIBOData.find(glid);
  mIBOData.erase(iter);

  GL(glDeleteBuffers(1, &glid));
}

GLuint IBOMan::hasIBO(const std::string& assetName) const
{
  for (auto it = mIBOData.begin(); it != mIBOData.end(); ++it)
  {
    if (it->second.assetName == assetName)
    {
      return it->first;
    }
  }
  return 0;
}

const IBOMan::IBOData& IBOMan::getIBOData(const std::string& assetName) const
{
  for (auto it = mIBOData.begin(); it != mIBOData.end(); ++it)
  {
    if (it->second.assetName == assetName)
    {
      return it->second;
    }
  }

  throw std::runtime_error("IBOMan: Unable to find IBO data");
}

//------------------------------------------------------------------------------
// GARBAGE COLLECTION
//------------------------------------------------------------------------------

void IBOMan::runGCAgainstVaidIDs(const std::set<GLuint>& validKeys)
{
  // Every GLuint in validKeys should be in our map. If there is not, then
  // there is an error in the system, and it should be reported.
  // The reverse is not expected to be true, and is what we are attempting to
  // correct with this function.
  auto it = mIBOData.begin();
  for (const GLuint& id : validKeys)
  {
    // Find the key in the map, eliminating any keys that do not match the
    // current id along the way.
    while (it != mIBOData.end() && it->first < id)
    {
      //\cb std::cout << "IBO GC: " << it->second.assetName << std::endl;

      GLuint idToErase = it->first;
      mIBOData.erase(it++);
      GL(glDeleteBuffers(1, &idToErase));
    }

    if (it == mIBOData.end())
    {
      std::cerr << "runGCAgainstVaidIDs: terminating early, validKeys contains "
                << "elements not in IBO map." << std::endl;
      break;
    }

    // Check to see if the valid ids contain a component that is not in
    // mIBOData. If an object manages its own IBO, but still uses the IBO
    // component, this is not an error.
    if (it->first > id)
    {
      std::cerr << "runGCAgainstVaidIDs: validKeys contains elements not in the IBO map." << std::endl;
    }

    // Increment passed current validKey id.
    ++it;
  }

  while (it != mIBOData.end())
  {
    //\cb std::cout << "IBO GC: " << it->second.assetName << std::endl;

    GLuint idToErase = it->first;
    mIBOData.erase(it++);
    GL(glDeleteBuffers(1, &idToErase));
  }
}

class IBOGarbageCollector :
    public es::GenericSystem<false, IBO>
{
public:

  static const char* getName()    {return "ren:IBOGarbageCollector";}

  std::set<GLuint> mValidKeys;

  void preWalkComponents(es::ESCoreBase&) {mValidKeys.clear();}
  void postWalkComponents(es::ESCoreBase& core)
  {
    std::weak_ptr<IBOMan> im = core.getStaticComponent<StaticIBOMan>()->instance_;
    if (std::shared_ptr<IBOMan> man = im.lock()) {
      man->runGCAgainstVaidIDs(mValidKeys);
      mValidKeys.clear();
    }
    else
    {
      std::cerr << "IBOMan: StaticIBOMan not found is given core. Failed to run IBO GC." << std::endl;
    }
  }

  void execute(es::ESCoreBase&, uint64_t /* entityID */, const IBO* ibo) override
  {
    mValidKeys.insert(ibo->glid);
  }
};

void IBOMan::registerSystems(CPM_ES_ACORN_NS::Acorn& core)
{
  // Register the garbage collector.
  core.registerSystem<IBOGarbageCollector>();
}

void IBOMan::runGCCycle(CPM_ES_NS::ESCoreBase& core)
{
  IBOGarbageCollector gc;
  gc.walkComponents(core);
}

const char* IBOMan::getGCName()
{
  return IBOGarbageCollector::getName();
}

} // namespace ren


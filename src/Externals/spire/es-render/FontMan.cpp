#include <string>
#include <sstream>
#include <lodepng/lodepng.h>
#include <es-fs/Filesystem.hpp>
#include <es-fs/fscomp/StaticFS.hpp>
#include <entity-system/GenericSystem.hpp>
#include "FontMan.hpp"
#include "comp/Font.hpp"
#include "comp/FontPromise.hpp"
#include "comp/StaticFontMan.hpp"
#include "comp/StaticTextureMan.hpp"

namespace es = CPM_ES_NS;
namespace fs = CPM_ES_FS_NS;;

namespace ren {

FontMan::FontMan(int numRetries) :
    mNewUnfulfilledAssets(false),
    mNumRetries(numRetries),
    mLastFontID(0)
{
}

FontMan::~FontMan()
{
}

void FontMan::loadFont(
    CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
    const std::string& assetName)
{
  if (buildComponent(core, entityID, assetName) == false)
  {
    FontPromise newPromise;
    newPromise.requestInitiated = false;
    newPromise.setAssetName(assetName.c_str());

    core.addComponent(entityID, newPromise);
  }
}

void FontMan::requestFont(CPM_ES_NS::ESCoreBase& core, const std::string& assetName,
                             int32_t numRetries)
{
  fs::StaticFS* sfs = core.getStaticComponent<fs::StaticFS>();

  /// \todo Get rid of this code when we switch to the new emscripten backend.
  ///       std::bind is preferable to cooking up a lambda. See ShaderMan
  ///       for functional examples.
  es::ESCoreBase* refPtr = &core;
  auto callbackLambda = [this, numRetries, refPtr](
      const std::string& asset, bool error, size_t bytesRead, uint8_t* buffer)
  {
    loadFontCB(asset, error, bytesRead, buffer, numRetries, *refPtr);
  };

  // Ensure there is a file extension on the asset name. The texture loader
  // does also accept requests for png.
  std::string::size_type dotIdx = assetName.rfind('.');
  std::string fullAssetName = assetName;
  if (dotIdx == std::string::npos)
  {
    fullAssetName = assetName + ".fnt";
  }
  sfs->instance->readFile(assetName, callbackLambda);
}

void FontMan::loadFontCB(const std::string& assetName, bool error,
                         size_t bytesRead, uint8_t* buffer,
                         int32_t numRetries, CPM_ES_NS::ESCoreBase& core)
{
  if (!error)
  {
    // Generate a new ID for this font.
    ++mLastFontID;
    uint64_t fontID = mLastFontID;

    // No error. We have loaded the font file and are ready to load it up.
    // Remember, we are toying with the idea that the texture will not be
    // applied onto the object itself, but onto a ficticious font entity which
    // will be responsible for holding onto the texture until it is garbage
    // collected.
    mIDToFont.insert(std::make_pair(fontID, FontInfo(fontID, assetName)));

    auto it = mIDToFont.find(fontID);
    if (it != mIDToFont.end())
    {
      BMFont& font = it->second.fontInfo;
      font.loadFromBuffer(buffer, bytesRead);
    }
    else
    {
      std::cerr << "FontMan: Failed promise for " << assetName 
                << ". Unable to find newly inserted value!" << std::endl;
    }
  }
  else
  {
    if (numRetries > 0)
    {
      // Reattempt the request
      --numRetries;
      requestFont(core, assetName, numRetries);
    }
    else
    {
      std::cerr << "FontMan: Failed promise for " << assetName << std::endl;
    }
  }
}

uint64_t FontMan::getIDForAsset(const char* assetName) const
{
  // Search through our map to find the asset with the appropriate name.
  for (auto it = mIDToFont.begin(); it != mIDToFont.end(); ++it)
  {
    if (it->second.assetName == assetName)
    {
      return it->second.fontID;
    }
  }

  return 0;
}

std::string FontMan::getAssetFromID(uint64_t id) const
{
  auto it = mIDToFont.find(id);
  if (it != mIDToFont.end())
  {
    return it->second.assetName;
  }
  else
  {
    return "";
  }
}

const BMFont& FontMan::getBMFontInfo(uint64_t id) const
{
  auto it = mIDToFont.find(id);
  if (it != mIDToFont.end())
  {
    return it->second.fontInfo;
  }
  else
  {
    throw std::runtime_error("FontMan: unable to find font.");
  }
}

const FontMan::FontInfo& FontMan::getFontInfo(uint64_t id)
{
  auto it = mIDToFont.find(id);
  if (it != mIDToFont.end())
  {
    return it->second;
  }
  else
  {
    throw std::runtime_error("FontMan: unable to find font.");
  }
}

bool FontMan::buildComponent(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                             const std::string& assetName)
{
  uint64_t id = getIDForAsset(assetName.c_str());
  if (id != 0)
  {
    // Go ahead and add a new component for the entityID. If this is the
    // last promise to fulfill, then systems should automatically start
    // rendering the data.
    Font component;
    component.fontID = id;
    core.addComponent(entityID, component);

    // Ensure we look for the texture in the correct directory (the same
    // directory as the font itself).
    std::string::size_type slashIdx = assetName.rfind('/');
    std::string rootAssetName = "";
    if (slashIdx != std::string::npos)
    { 
      rootAssetName = assetName.substr(0, slashIdx + 1);
    }

    std::weak_ptr<TextureMan> tm =
            core.getStaticComponent<StaticTextureMan>()->instance_;
    if (std::shared_ptr<TextureMan> texMan = tm.lock()) {
        const FontMan::FontInfo& info = getFontInfo(id);
        const BMFont::PageBlock& pageBlock = info.fontInfo.getPageBlock();
        for (size_t i = 0; i < pageBlock.pages.size(); ++i)
        {
          // Extract texture name and replace filename in asset with this
          // texture name? Or should we just replace this asset's file
          // extension and make that the thing?
          std::string textureName = rootAssetName + pageBlock.pages[i];

          // Ensure the texture has the appropriate itx extension.
          std::string::size_type dotIdx = textureName.rfind('.');
          if (dotIdx != std::string::npos)
          {
            textureName = textureName.substr(0, dotIdx);
          }
          textureName += ".itx";

          std::stringstream ss;
          ss << "uTX" << i;
          texMan->loadTexture(core, entityID, textureName, static_cast<int32_t>(i), ss.str());
        }

        return true;
    }
    return false;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------------------
// PROMISE FULFILLMENT
//------------------------------------------------------------------------------

class FontPromiseFulfillment :
    public es::GenericSystem<true,
                             FontPromise,
                             StaticFontMan>
{
public:

  static const char* getName() {return "ren:FontPromiseFulfillment";}

  /// This is only ever touched if requestInitiated is false for any component.
  /// It is updated during traversal and checked at the end of traversal
  /// against requested assets to see if the asset is already being loaded.
  std::set<std::string> mAssetsAwaitingRequest;

  /// Names of the assets currently being processed for which an additional
  /// request should not be attempted.
  std::set<std::string> mAssetsAlreadyRequested;

  void preWalkComponents(es::ESCoreBase&)
  {
    mAssetsAwaitingRequest.clear();
    mAssetsAlreadyRequested.clear();
  }

  void postWalkComponents(es::ESCoreBase& core)
  {
    StaticFontMan* man = core.getStaticComponent<StaticFontMan>();
    if (man == nullptr)
    {
      std::cerr << "Unable to complete font fulfillment. There is no StaticFontMan." << std::endl;
      return;
    }
    std::weak_ptr<FontMan>  fm = man->instance_;
    if (std::shared_ptr<FontMan> fontMan = fm.lock()) {
        fontMan->mNewUnfulfilledAssets = false;

        if (mAssetsAwaitingRequest.size() > 0)
        {
          std::set<std::string> assetsWithNoRequest;
          // Compute set difference and initiate requests for appropriate
          // components.
          std::set_difference(mAssetsAwaitingRequest.begin(), mAssetsAwaitingRequest.end(),
                              mAssetsAlreadyRequested.begin(), mAssetsAlreadyRequested.end(),
                              std::inserter(assetsWithNoRequest, assetsWithNoRequest.end()));

          for (const std::string& asset : assetsWithNoRequest)
          {
            fontMan->requestFont(core, asset, fontMan->mNumRetries);
          }
        }
    }
  }

  void groupExecute(es::ESCoreBase& core, uint64_t entityID,
               const es::ComponentGroup<FontPromise>& promisesGroup,
               const es::ComponentGroup<StaticFontMan>& fontManGroup) override
  {
    std::weak_ptr<FontMan> fm = fontManGroup.front().instance_;
    if (std::shared_ptr<FontMan> fontMan = fm.lock()) {

        CPM_ES_CEREAL_NS::CerealCore* ourCorePtr =
                dynamic_cast<CPM_ES_CEREAL_NS::CerealCore*>(&core);
        if (ourCorePtr == nullptr)
        {
          std::cerr << "Unable to execute font promise fulfillment. Bad cast." << std::endl;
          return;
        }
        CPM_ES_CEREAL_NS::CerealCore& ourCore = *ourCorePtr;

        int index = 0;
        for (const FontPromise& p : promisesGroup)
        {
          // Check to see if this promise has been fulfilled. If it has, then
          // remove it and create the appropriate component for the indicated
          // entity.
          if (fontMan->buildComponent(ourCore, entityID, p.assetName))
          {
            // Remove this promise, and add a font component to this promises'
            // entityID. It is safe to remove components while we are using a
            // system - addition / removal / modification doesn't happen until
            // a renormalization step.
            ourCore.removeComponentAtIndexT<FontPromise>(entityID, index);
          }
          else
          {
            // The asset has not be loaded. Check to see if a request has
            // been initiated for the assets; if not, then run the request.
            // (this can happen when we serialize the game while we are
            // still waiting for assets).
            if (p.requestInitiated == false)
            {
              // Modify pre-existing promise to indicate that we are following
              // up with the promise. But, we don't initiate the request yet
              // since another promise may have already done so. We wait until
              // postWalkComponents to make a decision.
              FontPromise newPromise = p;
              newPromise.requestInitiated = true;
              promisesGroup.modify(newPromise, static_cast<size_t>(index));

              mAssetsAwaitingRequest.insert(std::string(newPromise.assetName));
            }
            else
            {
              mAssetsAlreadyRequested.insert(std::string(p.assetName));
            }
          }

          ++index;
        }
      }
  }
};

const char* FontMan::getPromiseSystemName()
{
  return FontPromiseFulfillment::getName();
}

//------------------------------------------------------------------------------
// GARBAGE COLLECTION
//------------------------------------------------------------------------------

void FontMan::runGCAgainstVaidIDs(const std::set<uint64_t>& validKeys)
{
  if (mNewUnfulfilledAssets)
  {
    std::cerr << "FontMan: Terminating garbage collection. Orphan assets that"
              << " have yet to be associated with entity ID's would be GC'd" << std::endl;
    return;
  }

  // Every uint64_t in validKeys should be in our map. If there is not, then
  // there is an error in the system, and it should be reported.
  // The reverse is not expected to be true, and is what we are attempting to
  // correct with this function.
  auto it = mIDToFont.begin();
  for (const uint64_t& id : validKeys)
  {
    // Find the key in the map, eliminating any keys that do not match the
    // current id along the way. We iterate through both the map and the set
    // in an ordered fashion.
    while (it != mIDToFont.end() && it->first < id)
    {
      std::cout << "FontMan GC: " << it->second.assetName << std::endl;
      mIDToFont.erase(it++);
    }

    if (it == mIDToFont.end())
    {
      std::cerr << "runGCAgainstVaidIDs: terminating early, validKeys contains "
                << "elements not in texture map." << std::endl;
      break;
    }

    // Check to see if the valid ids contain a component that is not in
    // mIDToFont. If an object manages its own texture, but still uses the
    // texture component, this is not an error.
    if (it->first > id)
    {
      std::cerr << "runGCAgainstVaidIDs: validKeys contains elements not in the texture map." << std::endl;
    }

    ++it;
  }

  while (it != mIDToFont.end())
  {
    std::cout << "FontMan GC: " << it->second.assetName << std::endl;
    mIDToFont.erase(it++);
  }
}

class FontGarbageCollector :
    public es::GenericSystem<false, Font>
{
public:

  static const char* getName() {return "ren:FontGarbageCollector";}

  std::set<uint64_t> mValidKeys;

  void preWalkComponents(es::ESCoreBase&) {mValidKeys.clear();}

  void postWalkComponents(es::ESCoreBase& core)
  {
    StaticFontMan* man = core.getStaticComponent<StaticFontMan>();
    if (man == nullptr)
    {
      std::cerr << "Unable to complete texture garbage collection. There is no StaticFontMan." << std::endl;
      return;
    }
    std::weak_ptr<FontMan>  fm = man->instance_;

    if (std::shared_ptr<FontMan> fontMan = fm.lock()) {
        fontMan->runGCAgainstVaidIDs(mValidKeys);
        mValidKeys.clear();
    }
  }

  void execute(es::ESCoreBase&, uint64_t /* entityID */, const Font* font) override
  {
    mValidKeys.insert(font->fontID);
  }
};

const char* FontMan::getGCName()
{
  return FontGarbageCollector::getName();
}

void FontMan::registerSystems(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<FontPromiseFulfillment>();
  core.registerSystem<FontGarbageCollector>();
}

void FontMan::runGCCycle(CPM_ES_NS::ESCoreBase& core)
{
  FontGarbageCollector gc;
  gc.walkComponents(core);
}

} // namespace ren


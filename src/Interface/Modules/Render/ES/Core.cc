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


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include "Core.h"

#include <es-fs/Registration.hpp>
#include <es-general/Registration.hpp>
#include <es-general/comp/StaticGlobalTime.hpp>
#include <es-render/Registration.hpp>
#include <es-render/comp/StaticGLState.hpp>

#include "Registration.h"

namespace SCIRun {
namespace Render {

ESCore::ESCore() : mCurrentTime(0.0f), r_(0.0f), g_(0.0f), b_(0.0f), a_(0.0f)
{
  // Register common systems.
  gen::registerAll(*this);
  spire::registerAll(*this);
  ren::registerAll(*this);
  rendererRegisterAll(*this);

  // You should set your default OpenGL state here before we create the
  // default static component GLState.
  mDefaultGLState.setFrontFace(GL_CW);
  mDefaultGLState.setCullFace(GL_BACK);
  mDefaultGLState.setCullFaceEnable(true);

  // Register defaultGLState.
  ren::StaticGLState esGLState;
  esGLState.state = mDefaultGLState;
  addStaticComponent(esGLState);
}

ESCore::~ESCore()
{
}

std::string ESCore::toString(std::string prefix) const
{
  std::string output = prefix + "ES_CORE:\n";
  prefix += "  ";

  output += prefix + "ComponentContainers: " + std::to_string(mComponents.size()) + "\n";
  for(auto& comp : mComponents)
    if(comp.second->getNumComponents() >= 0)
    {
      output += prefix + "Container: " + std::to_string(comp.first)
        + "  Components: " + std::to_string(comp.second->getNumComponents());
      if(mComponentIDNameMap.find(comp.first) != mComponentIDNameMap.end())
        output += "  Name: \"" + mComponentIDNameMap.at(comp.first) + "\"";
      else
        output += "  Name: not found ";
      output += "\n" + comp.second->toString(prefix);
    }
  output += "\n";

  output += prefix + "KernelSystems: " + std::to_string(mKernelSystems.size()) + "\n";
  for(auto& name: mKernelSystems)
    output += prefix + "  Name: " + name + "\n";
  output+= "\n";

  output += prefix + "UserSystems: " + std::to_string(mUserSystems.size()) + "\n";
  for(auto& name: mUserSystems)
    output += prefix + "  Name: " + name + "\n";
  output += "\n";

  output += prefix + "GarbageCollectorSystems: " + std::to_string(mGarbageCollectorSystems.size()) + "\n";
  for(auto& name: mGarbageCollectorSystems)
    output+= prefix + "  Name: " + name + "\n";
  output += "\n";

  auto systems = mSystems.get();
  output += systems->toString(prefix) + "\n";

  output += prefix + "Current Time: " + std::to_string(static_cast<uint64_t>(mCurrentTime * 1000.0)) + "\n";

  return output;
}

bool ESCore::hasGeomPromise() const
{
  for(auto& comp : mComponents)
  {
    if(comp.second->getNumComponents() > 0)
    {
      if(mComponentIDNameMap.find(comp.first) != mComponentIDNameMap.end() &&
         mComponentIDNameMap.at(comp.first) == "ren:GeomPromise")
        return true;
    }
  }
  return false;
}

bool ESCore::hasShaderPromise() const
{
  for(auto& comp : mComponents)
  {
    if(comp.second->getNumComponents() > 0)
    {
      if(mComponentIDNameMap.find(comp.first) != mComponentIDNameMap.end() &&
         mComponentIDNameMap.at(comp.first) == "ren:ShaderPromiseVF")
        return true;
    }
  }
  return false;
}

void ESCore::executeWithoutAdvancingClock()
{
  execute(0);
}

void ESCore::execute(double constantFrameTime)
{
  mCurrentTime += constantFrameTime;
  // Update the current static time component before renormalization.
  {
    gen::StaticGlobalTime globalTime;
    globalTime.globalTime = mCurrentTime;
    globalTime.deltaTime  = constantFrameTime;

    // Modify 'input'. If it doesn't already exist in the system, create it.
    gen::StaticGlobalTime* esGlobalTime = getStaticComponent<gen::StaticGlobalTime>();
    if (esGlobalTime == nullptr) addStaticComponent(globalTime);
    else *esGlobalTime = globalTime;
  }

  // Ensure all systems are appropriately added and removed.
  renormalize(true);
  mSystems->renormalize();

  // Perform garbage collection if requested and safe
  if(runGC)
  {
    if(!hasGeomPromise())
      runCompleteGC();
    runGC = false;
  }

  // Perform debug serialization here. You can save the frame here as well.
  // Might be useful for debugging.

  // Only perform OpenGL initialization steps if we have a valid and complete
  // frame buffer. We need to ensure that the core executes every frame however.
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
  {
    // Reset the GL state (we shouldn't really need to do this, but we will anyways).
    mDefaultGLState.apply();
    glClearColor(r_, g_, b_, a_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // Perform execution of systems.
  uint64_t timeInMS = static_cast<uint64_t>(mCurrentTime * 1000.0);
  mSystems->runSystems(*this, timeInMS);
}

void ESCore::setBackgroundColor(float r, float g, float b, float a)
{
  r_ = r;
  g_ = g;
  b_ = b;
  a_ = a;
}

} // namespace Render
} // namespace SCIRun

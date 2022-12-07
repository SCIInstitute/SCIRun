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


#ifndef SPIRE_COMPONENT_RENDER_FONT_HPP
#define SPIRE_COMPONENT_RENDER_FONT_HPP

#include <es-log/trace-log.h>
#include <es-cereal/ComponentSerialize.hpp>

#include <gl-platform/GLPlatform.hpp>
#include <es-cereal/CerealCore.hpp>
#include "FontPromise.hpp"
#include "StaticFontMan.hpp"
#include <spire/scishare.h>

namespace ren {

struct Font
{
  // -- Data --
  uint64_t fontID;

  // -- Functions --
  Font()
  {
    fontID = 0;
  }

  static const char* getName() {return "ren:Font";}

  bool serialize(spire::ComponentSerialize& s, uint64_t entityID)
  {
    // The logic below ensures we deserialize with promises, not with actual
    // font assets (which wouldn't make sense with OpenGL assets).
    if (s.isDeserializing())
    {
      std::string assetName;
      s.serialize("name", assetName);

      // Build font promise.
      FontPromise newPromise;
      newPromise.requestInitiated = false;
      newPromise.setAssetName(assetName.c_str());

      spire::CerealCore* core = dynamic_cast<spire::CerealCore*>(&s.getCore());
      if (core != nullptr)
      {
        core->addComponent(entityID, newPromise);
      }
      else
      {
        std::cerr << "Font.hpp bad cast." << std::endl;
      }

      return false; // We do not want to add this font component back into the components.
                    // Instead we rely on the font promise we created above.
    }
    else
    {
      spire::CerealCore* core = dynamic_cast<spire::CerealCore*>(&s.getCore());
      if (core != nullptr)
      {
        StaticFontMan* staticFontMan = core->getStaticComponent<StaticFontMan>();
        std::weak_ptr<FontMan> fontMan = staticFontMan->instance_;

        // Find the asset name associated with our fontID and serialize it out.
        if (std::shared_ptr<FontMan> fm = fontMan.lock()) {
            std::string assetName = fm->getAssetFromID(fontID);
            s.serialize("name", assetName);
            return true;
        }
        return false;
      }
      else
      {
        std::cerr << "Font.hpp bad cast." << std::endl;
        return false;
      }

      return true;
    }
  }
};

} // namespace ren

#endif

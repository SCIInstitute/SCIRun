/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#ifndef MODULES_FIELDS_BOXWIDGETTYPES_H
#define MODULES_FIELDS_BOXWIDGETTYPES_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Modules/Fields/share.h>

namespace SCIRun 
{

  class SCISHARE GeometryPortInterface
  {

  };

  class SCISHARE BoxWidgetInterface
  {
  public:
    virtual ~BoxWidgetInterface() {}

    //TODO: EMBB only uses this type for position information. Remaining methods are being commented out until they are actually implemented.

    //TODO: need conversion to GPI type above, maybe
    //virtual void connect(Dataflow::Networks::OutputPortHandle port) = 0;
    //virtual void setRestrictX(bool restrict) = 0;
    //virtual void setRestrictY(bool restrict) = 0;
    //virtual void setRestrictZ(bool restrict) = 0;
    //virtual void setRestrictR(bool restrict) = 0;
    //virtual void setRestrictD(bool restrict) = 0;
    //virtual void setRestrictI(bool restrict) = 0;
    //virtual void unrestrictTranslation() = 0;
    //virtual void restrictTranslationXYZ() = 0;
    //virtual void restrictTranslationRDI() = 0;
    virtual void setPosition(const Core::Geometry::Point&, const Core::Geometry::Point&, const Core::Geometry::Point&, const Core::Geometry::Point&) = 0;
    virtual void getPosition(Core::Geometry::Point&, Core::Geometry::Point&, Core::Geometry::Point&, Core::Geometry::Point&) const = 0;
    //virtual void setScale(double scale) = 0;
    //virtual void setCurrentMode(int mode) = 0;
  };

}

#endif

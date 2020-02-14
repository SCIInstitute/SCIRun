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


#ifndef MODULES_LEGACY_FIELDS_INTERFACEWITHTETGENIMPL_H__
#define MODULES_LEGACY_FIELDS_INTERFACEWITHTETGENIMPL_H__

#include <Dataflow/Network/Module.h>
#include <deque>
#include <Modules/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Fields {

      //TODO: convert to proper algo class
      struct SCISHARE InterfaceWithTetGenInput
      {
        InterfaceWithTetGenInput();
        bool piecewiseFlag_;            // -p
        bool assignFlag_;               // -A
        bool setNonzeroAttributeFlag_;  // -AA
        bool suppressSplitFlag_;        // -Y
        bool setSplitFlag_;             // -YY
        bool qualityFlag_;              // -q
        bool setRatioFlag_;             // -q
        bool volConstraintFlag_;        // -a
        bool setMaxVolConstraintFlag_;  // -a
        double minRadius_;
        double maxVolConstraint_;
        bool detectIntersectionsFlag_;  // -d
        std::string moreSwitches_;          // additional flags

        std::string fillCommandOptions(bool addPoints) const;
      };

      namespace detail
      {
        class InterfaceWithTetGenImplImpl;
      }

      class SCISHARE InterfaceWithTetGenImpl
      {
      public:
        InterfaceWithTetGenImpl(Dataflow::Networks::Module* mod, const InterfaceWithTetGenInput& input);
        FieldHandle runImpl(const std::deque<FieldHandle>& surfaces, FieldHandle points, FieldHandle region_attribs) const;
      private:
        boost::shared_ptr<detail::InterfaceWithTetGenImplImpl> impl_;
        InterfaceWithTetGenInput inputFlags_;
      };
    }
  }
}

#endif

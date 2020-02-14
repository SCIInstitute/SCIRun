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


/// @todo Documentation Modules/Visualization/ShowString.h

#ifndef MODULES_VISUALIZATION_SHOW_STRING_H
#define MODULES_VISUALIZATION_SHOW_STRING_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {

  namespace Core {
    namespace Algorithms {
      namespace Visualization {

        ALGORITHM_PARAMETER_DECL(TextRed);
        ALGORITHM_PARAMETER_DECL(TextGreen);
        ALGORITHM_PARAMETER_DECL(TextBlue);
        ALGORITHM_PARAMETER_DECL(TextAlpha);
        ALGORITHM_PARAMETER_DECL(FontName);
        ALGORITHM_PARAMETER_DECL(FontSize);
        ALGORITHM_PARAMETER_DECL(PositionType);
        ALGORITHM_PARAMETER_DECL(FixedHorizontal);
        ALGORITHM_PARAMETER_DECL(FixedVertical);
        ALGORITHM_PARAMETER_DECL(CoordinateHorizontal);
        ALGORITHM_PARAMETER_DECL(CoordinateVertical);
      }
    }
  }

namespace Modules {
namespace Visualization {

  class SCISHARE ShowString : public SCIRun::Dataflow::Networks::GeometryGeneratingModule,
    public Has1InputPort<StringPortTag>,
    public Has1OutputPort<GeometryPortTag>
  {
  public:
    ShowString();
    virtual void execute() override;
    virtual void setStateDefaults() override;
    INPUT_PORT(0, String, String);
    OUTPUT_PORT(0, RenderedString, GeometryObject);
    MODULE_TRAITS_AND_INFO(ModuleHasUI)
  private:
    Core::Datatypes::GeometryBaseHandle buildGeometryObject(const std::string& text);
    std::tuple<double, double> getTextPosition();
    void processWindowResizeFeedback(const Core::Datatypes::ModuleFeedback& var);
    static bool containsDescenderLetter(const std::string& text);
    boost::shared_ptr<class TextBuilder> textBuilder_;
    std::tuple<int,int> lastWindowSize_ { 450, 1000 };
    bool needReexecute_{ true }, executedOnce_{ false };
  };
}}}

#endif

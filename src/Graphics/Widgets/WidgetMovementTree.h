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

#ifndef Graphics_Graphics_Widgets_WidgetTreeBuilder_H
#define Graphics_Graphics_Widgets_Widget_H

#include <Graphics/Widgets/WidgetParameters.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun
{
  namespace Graphics
  {
    namespace Datatypes
    {
      //TODO: ideas
      class SCISHARE ResultantTransformationWidgetMap
      {
      private:
        std::map<WidgetMovement, WidgetHandle> subscribersByResultantMovement_;
      };

      class SCISHARE RootMovementSubscribers
      {
      private:
        std::map<WidgetMovement, ResultantTransformationWidgetMap> subscribedResultantTransformablesByRootMovement_;
      };

      class SCISHARE WidgetMovementTree
      {
      private:
        WidgetHandle rootWidget_;
        RootMovementSubscribers subscribers_;
      };

      class SCISHARE WidgetMovementTreeBuilder
      {
      public:
        explicit WidgetMovementTreeBuilder(WidgetHandle w);
      };
    }
  }
}

#endif

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


#ifndef INTERFACE_APPLICATION_FloatingDockWindow_H
#define INTERFACE_APPLICATION_FloatingDockWindow_H

#include <QWidget>

// Platform-specific window behavior for floating dock widgets.
namespace SCIRun {
namespace Gui {
namespace FloatingDockWindow {

#ifdef __APPLE__
  // macOS greys out minimize/zoom unless the hint is explicit (#2748). Elsewhere an
  // explicit hint drops Qt's default title/system-menu/close hints.
  const Qt::WindowFlags flags = Qt::Window | Qt::WindowMinMaxButtonsHint;
  constexpr bool setFlagsOnCreate = true;
  constexpr bool restoreWhenMinimized = false;
#else
  const Qt::WindowFlags flags = Qt::Window;
  constexpr bool setFlagsOnCreate = false;
  // Docks are owned windows, which minimize with no taskbar entry; the module UI button has to bring them back.
  constexpr bool restoreWhenMinimized = true;
#endif

  inline void applyFlags(QWidget* dock)
  {
    dock->setWindowFlags(flags);
  }

  inline void applyFlagsOnCreate(QWidget* dock)
  {
    if (setFlagsOnCreate)
      applyFlags(dock);
  }

  // A minimized window is not hidden, but there is nothing on screen to click.
  inline bool isClosed(const QWidget* dock)
  {
    return dock->isHidden() || (restoreWhenMinimized && dock->isMinimized());
  }

  inline void restoreIfMinimized(QWidget* dock)
  {
    if (restoreWhenMinimized)
      dock->setWindowState(dock->windowState() & ~Qt::WindowMinimized);
  }

}
}
}
#endif

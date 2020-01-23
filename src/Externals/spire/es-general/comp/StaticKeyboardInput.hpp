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


#ifndef SPIRE_ES_GENERAL_STATIC_KEYBOARD_INPUT_HPP
#define SPIRE_ES_GENERAL_STATIC_KEYBOARD_INPUT_HPP

#include <es-log/trace-log.h>
#include <spire/scishare.h>

/// \note If you want to enter text from the keyboard, you should use the
///       character callbacks (glfw) not the keyboard input.
///       For example, you'll never see StaticKeyboardInput present on
///       mobile devices, but StaticCharacterInput will be. Keyboard input
///       doesn't make sense on mobile devices for games, but character input
///       through the systems on-screen keyboard does make sense.

namespace gen {

struct StaticKeyboardInput
{
  enum SpecialKey
  {
    KEY_NO_SPECIAL,
    KEY_UNKNOWN,
    KEY_SPACE,   // Space key
    KEY_ESC,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_LCTRL,
    KEY_RCTRL,
    KEY_LALT,
    KEY_RALT,
    KEY_TAB,
    KEY_ENTER,
    KEY_BACKSPACE,
    KEY_INSERT,
    KEY_DEL,
    KEY_LSUPER,
    KEY_RSUPER,
    KEY_MENU,
    KEY_COMMA,
    KEY_APOSTROPHE
  };

  enum KeyAction
  {
    PRESS,
    RELEASE
  };

  // If the character is not one of the special keys given above, then
  // it is a character such as 'a'.
  struct KeyEvent
  {
    SpecialKey  special;  // If a special character, then something other than KEY_NO_SPECIAL.
    int         keyCode;  // Always set. It is the character code.
    KeyAction   action;   // Either press or release.

    void clear()
    {
      special = KEY_NO_SPECIAL;
      keyCode = 0;
      action = RELEASE;
    }
  };

  // -- Data --
  static const int MAX_KEY_EVENT_BUFFER = 8;
  KeyEvent  events[MAX_KEY_EVENT_BUFFER];
  int32_t   eventsSize;

  // -- Functions --
  StaticKeyboardInput()
  {
    clearKeyEventBuffer();
  }

  static const char* getName() {return "gen:StaticKeyboardInput";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize keyboard input
    return true;
  }

  int32_t size() const            {return eventsSize;}
  KeyEvent* begin()               {return events;}
  KeyEvent* end()                 {return &events[eventsSize];}
  const KeyEvent* cbegin() const  {return events;}
  const KeyEvent* cend() const    {return &events[eventsSize];}

  void clearKeyEventBuffer()
  {
    // We specifically do not clear the current mouse pos.
    eventsSize = 0;
    for (int i = 0; i < MAX_KEY_EVENT_BUFFER; ++i)
    {
      events[i].clear();
    }
  }

  void pushKey(int keycode, SpecialKey sp, KeyAction action)
  {
    if (eventsSize < MAX_KEY_EVENT_BUFFER)
    {
      events[eventsSize].special  = sp;
      events[eventsSize].keyCode  = keycode;
      events[eventsSize].action   = action;
      ++eventsSize;
    }
    else
    {
      logRendererError("Eating keyboard event. Buffer too small.");
    }
  }
};

} // namespace gen

#endif

#ifndef IAUNS_ES_GENERAL_STATIC_MOUSE_INPUT_HPP
#define IAUNS_ES_GENERAL_STATIC_MOUSE_INPUT_HPP

#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace gen {

// This component will *not* be present when there is no mouse input available
// for the system. You can check other input sources such as touch input
// or keyboard input.
struct StaticMouseInput
{
  enum MouseButton
  {
    NO_BUTTON   = 0,
    LEFT        = 1,
    RIGHT       = 2,
    MIDDLE      = 3,
    NUM_BUTTONS = 4,
  };

  enum MouseEventID
  {
    MOUSE_MOVE,
    MOUSE_DOWN,
    MOUSE_UP,
    MOUSE_WHEEL,
  };

  struct MouseEvent
  {
    MouseEventID  event;      // Always populated
    MouseButton   button;     // Only valid on DOWN and UP events.
    glm::vec2     mousePos;   // Always populated
    int           mouseWheel; // Only valid on wheel event.
    MouseEventID  btnState[NUM_BUTTONS]; // Always populated.
    bool          handled;    // Set to true by any system that has handled the
                              // event. No further processing should be done.

    void clear()
    {
      event       = MOUSE_UP;
      button      = NO_BUTTON;
      mousePos    = glm::vec2(0.0f, 0.0f);
      mouseWheel  = 0;
      handled     = false;
      for (int i = 0; i < NUM_BUTTONS; ++i)
      {
        btnState[i] = MOUSE_UP;
      }
    }
  };

  // -- Data --
  // Mouse position data is from [0..1] on both axes.
  // The origin is located in the top left of the screen with increasing values
  // running right and down along the axes.
  static const int MAX_MOUSE_EVENT_BUFFER = 8;

  MouseEvent    events[MAX_MOUSE_EVENT_BUFFER];
  int32_t       eventsSize;

  // -- Functions --
  StaticMouseInput()
  {
    clearMouseEventBuffer();
    curMousePos = glm::vec2(0.0f, 0.0f);
    for (int i = 0; i < NUM_BUTTONS; ++i)
    {
      curBtnState[i] = MOUSE_UP;
    }
  }

  int32_t size() const              {return eventsSize;}
  const MouseEvent* cbegin() const  {return events;}
  const MouseEvent* cend() const    {return &events[eventsSize];}

  void clearMouseEventBuffer()
  {
    // We specifically do not clear the current mouse pos.
    eventsSize = 0;
    for (int i = 0; i < MAX_MOUSE_EVENT_BUFFER; ++i)
    {
      events[i].clear();
    }
  }

  void pushMouseMove(const glm::vec2& pos)
  {
    curMousePos = pos;
    if (eventsSize < MAX_MOUSE_EVENT_BUFFER)
    {
      events[eventsSize].event = MOUSE_MOVE;
      events[eventsSize].mousePos = curMousePos;
      copyCommonVarsToEvent(eventsSize);
      ++eventsSize;
    }
    else
    {
      std::cerr << "Eating mouse event. Buffer too small." << std::endl;
    }
  }

  void pushMouseDown(MouseButton btn)
  {
    if (eventsSize < MAX_MOUSE_EVENT_BUFFER)
    {
      int btnIndex = static_cast<int>(btn);
      curBtnState[btnIndex] = MOUSE_DOWN;

      events[eventsSize].event = MOUSE_DOWN;
      events[eventsSize].button = btn;
      copyCommonVarsToEvent(eventsSize);
      ++eventsSize;
    }
    else
    {
      std::cerr << "Eating mouse event. Buffer too small." << std::endl;
    }
  }

  void pushMouseUp(MouseButton btn)
  {
    if (eventsSize < MAX_MOUSE_EVENT_BUFFER)
    {
      int btnIndex = static_cast<int>(btn);
      curBtnState[btnIndex] = MOUSE_UP;

      events[eventsSize].event = MOUSE_UP;
      events[eventsSize].button = btn;
      copyCommonVarsToEvent(eventsSize);
      ++eventsSize;
    }
    else
    {
      std::cerr << "Eating mouse event. Buffer too small." << std::endl;
    }
  }

  void pushMouseWheel(int wheel)
  {
    if (eventsSize < MAX_MOUSE_EVENT_BUFFER)
    {
      events[eventsSize].event = MOUSE_WHEEL;
      events[eventsSize].mouseWheel = wheel;
      copyCommonVarsToEvent(eventsSize);
      ++eventsSize;
    }
    else
    {
      std::cerr << "Eating mouse event. Buffer too small." << std::endl;
    }
  }

  // Calculates 'screen space coordinates'. Coordinates with the origin centered
  // in the middle of the screen with normal axes (+x left, +y up), and each
  // axis ranging from -1 to 1.
  static glm::vec2 calcScreenSpace(const glm::vec2& mousePos)
  {
    // Transform incoming mouse coordinates into screen space.
    glm::vec2 ss;
    ss.x = 2.0f * mousePos.x - 1.0f;
    ss.y = 2.0f * mousePos.y - 1.0f;

    // Rotation with flipped axes feels much more natural. It places it inside
    // the correct OpenGL coordinate system (with origin in the center of the
    // screen).
    ss.y = -ss.y;

    return ss;
  }

  static const char* getName() {return "gen:StaticMouseInput";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize mouse input
    return true;
  }

private:

  // Mouse state variables.
  glm::vec2     curMousePos;
  MouseEventID  curBtnState[NUM_BUTTONS];

  void copyCommonVarsToEvent(int id)
  {
    events[id].mousePos = curMousePos;
    for (int i = 0; i < NUM_BUTTONS; ++i)
    {
      events[id].btnState[i] = curBtnState[i];
    }
  }
};

} // namespace ren

#endif 

#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <stdint.h>

#include <glm/glm.hpp>

#include "Logger.hpp"

enum class ButtonState : bool { Down = false, Up };
enum class MouseAxis : bool { X, Y };


enum class GPButtonID {
   Invalid = SDL_CONTROLLER_BUTTON_INVALID,

   A = SDL_CONTROLLER_BUTTON_A,

   B = SDL_CONTROLLER_BUTTON_B,

   X = SDL_CONTROLLER_BUTTON_X,

   Y = SDL_CONTROLLER_BUTTON_Y,

   Back = SDL_CONTROLLER_BUTTON_BACK,

   Guide = SDL_CONTROLLER_BUTTON_GUIDE,

   Start = SDL_CONTROLLER_BUTTON_START,

   LeftStick = SDL_CONTROLLER_BUTTON_LEFTSTICK,

   RightStick = SDL_CONTROLLER_BUTTON_RIGHTSTICK,

   LeftShoulder = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,

   RightShoulder = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,

   DPadUp = SDL_CONTROLLER_BUTTON_DPAD_UP,

   DPadDown = SDL_CONTROLLER_BUTTON_DPAD_DOWN,

   DPadLeft = SDL_CONTROLLER_BUTTON_DPAD_LEFT,

   DPadRight = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,

   Max = SDL_CONTROLLER_BUTTON_MAX,
};
enum class GPAxisID {
   Invalid      = SDL_CONTROLLER_AXIS_INVALID,
   LeftX        = SDL_CONTROLLER_AXIS_LEFTX,
   LeftY        = SDL_CONTROLLER_AXIS_LEFTY,
   RightX       = SDL_CONTROLLER_AXIS_RIGHTX,
   RightY       = SDL_CONTROLLER_AXIS_RIGHTY,
   LeftTrigger  = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
   RightTrigger = SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
   Max          = SDL_CONTROLLER_AXIS_MAX
};

enum class AxisType {
   Keyboard = 0,
   MouseButton,
   GamepadButton,
   MouseAxis,
   GamepadAxis,

};

using KeyID            = SDL_Keycode;
using MouseButton      = uint8_t;
using GamepadID        = uint8_t;
using MouseButtonState = uint8_t;

struct AxisMapping {
   // No matter if it's key or mouseButton, they share the same location and size.
   AxisMapping(AxisType ty, KeyID id) : type{ty}, key{id} {}
   AxisMapping(AxisType ty, std::pair<GamepadID, GPButtonID> button) : type{ty}, gpButton{button} {}
   AxisMapping(AxisType ty, std::pair<GamepadID, GPAxisID> button) : type{ty}, gpAxis{button} {}
   AxisMapping(AxisType ty, MouseAxis ax) : type{ty}, mouseAxis{ax} {}

   AxisType type;

   union {
      KeyID                            key, mouseButton;
      std::pair<GamepadID, GPButtonID> gpButton;
      std::pair<GamepadID, GPAxisID>   gpAxis;
      MouseAxis                        mouseAxis;
   };
};


class Input {
  public:
   struct Axis {
      Axis(AxisMapping mapping) : mappedTo{mapping} {}
      float       cur, prev;
      AxisMapping mappedTo;
   };

   using AxisID      = uint16_t;  ///!> Direct axis lookup. If at all possible, cache this and only use this.
   using KeyCallback = void (*)(KeyID);


   Input(SDL_Window* window) : window{window}, running{true} {}

   bool shouldQuit() { return !running; }

   void update() {
      SDL_Event event;
      while (SDL_PollEvent(&event))
         if (event.type == SDL_QUIT)
            running = false;
   }

   // Forces a full update of every axis. Use sparingly.
   void fullUpdate() {
      // Todo: Perhaps have this take from an external state so we could theoretically multithread this part (assuming
      // enough axes).
      auto       kbstate = SDL_GetKeyboardState(nullptr);
      int        mouseX, mouseY;
      auto       mouseState = SDL_GetMouseState(&mouseX, &mouseY);
      glm::ivec2 windowDims;
      SDL_GetWindowSize(window, &windowDims.x, &windowDims.y);

      for (auto& axis : axes) {
         const auto& mapping = axis.mappedTo;

         axis.prev = axis.cur;

         switch (mapping.type) {
            case AxisType::Keyboard:
               if (kbstate[SDL_GetScancodeFromKey(mapping.key)])
                  axis.cur = 1.0f;
               else
                  axis.cur = 0.0f;
               break;

            case AxisType::MouseButton:
               if (mouseState & SDL_BUTTON(mapping.mouseButton))
                  axis.cur = 1.0f;
               else
                  axis.cur = 0.0f;
               break;

            case AxisType::GamepadButton:
               Logger::Debug("You haven't implemented Gamepads yet, doofus.");
               break;

            case AxisType::MouseAxis:
               if (mapping.mouseAxis == MouseAxis::X)
                  axis.cur = mouseX / static_cast<float>(windowDims.x);
               else
                  axis.cur = mouseY / static_cast<float>(windowDims.y);
               break;

            case AxisType::GamepadAxis:
               Logger::Debug("You haven't implemented this either");
               break;

            default:
               Logger::Debug("Attempted to use unknown AxisType ", static_cast<size_t>(mapping.type));
               break;
         }
      }
   }
   inline float     getAxis(const std::string& name) { return getAxis(axisNames[name]); }
   inline float     getAxis(AxisID id) { return axes[id].cur; }
   ButtonState      getRawKeyState(KeyID key) { SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(key)]; }
   ButtonState      getRawButtonState(GamepadID gp, GPButtonID button);  // TODO
   glm::ivec2       getMousePosition();
   MouseButtonState getMouseState();

   void registerKeyCallback(ButtonState whichState, KeyCallback callback);
   void unregisterKeyCallback(KeyCallback callback);

  private:
   SDL_Window*                             window;
   bool                                    running;
   std::unordered_map<std::string, AxisID> axisNames;  // AxisID indexes into axes
   std::vector<Axis>                       axes;
};

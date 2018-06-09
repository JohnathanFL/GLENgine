#include "Input.hpp"

InputEventData::~InputEventData() {}

bool Input::handleEvents() {
   SDL_Event e;
   while (SDL_PollEvent(&e)) {
      switch (e.type) {
         case SDL_QUIT:
            this->callAll(Event::Quit, InputEventData{});
            return false;
            break;

         case SDL_KEYDOWN:
            this->callAll(Event::Keydown, InputEventData{e.key});
            break;


         case SDL_KEYUP:
            this->callAll(Event::Keyup, InputEventData{e.key});
            break;

         default:
            break;
      }
   }


   return true;
}

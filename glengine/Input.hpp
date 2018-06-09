#pragma once
#include <typeindex>
#include <functional>

#include <SDL2/SDL.h>

#include "Events.hpp"

struct InputEventData : public CustomEventData {
  public:
   InputEventData() {
   }

   InputEventData(const SDL_KeyboardEvent& ev) {
      key = ev.keysym.sym;
   }
   virtual ~InputEventData() {
   }

   SDL_Keycode key;
};

class Input {
  public:
   Input(SDL_Window* window) {
      this->window = window;
   }

   bool handleEvents() {
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


   enum class Event : uint8_t {
      Keydown = 0,
      Keyup,
      Quit,

      COUNT
   };

   inline void addEvent(uint32_t adder, const Event& forWhat,
                        std::function<void(const EventData&)> handler) {
      registered[(uint8_t)forWhat][adder] = handler;
   }

  private:
   void callAll(const Event& ev, const InputEventData& data) {
      EventData event = {std::type_index(typeid(Input)), data};
      for (auto& funcPair : registered[(uint8_t)ev])
         funcPair.second(event);
   }


   SDL_Window* window;
   std::array<
       std::unordered_map<uint32_t, std::function<void(const EventData&)>>,
       (uint8_t)Event::COUNT>
       registered;
};

#pragma once
#include <functional>
#include <typeindex>

#include <SDL2/SDL.h>

#include "Events.hpp"

struct InputEventData : public CustomEventData {
  public:
   InputEventData() {}

   InputEventData(const SDL_KeyboardEvent& ev) { key = ev.keysym.sym; }
   virtual ~InputEventData();

   SDL_Keycode key;
};

class Input {
  public:
   Input(SDL_Window* window) { this->window = window; }

   bool handleEvents();


   enum class Event : uint8_t {
      Keydown = 0,
      Keyup,
      Quit,

      COUNT
   };

   inline void addEvent(uint32_t adder, const Event& forWhat,
                        std::function<void(const EventData&)> handler);

  private:
   inline void callAll(const Event& ev, const InputEventData& data);


   SDL_Window* window;
   std::array<
       std::unordered_map<uint32_t, std::function<void(const EventData&)>>,
       (uint8_t)Event::COUNT>
       registered;
};

void Input::addEvent(uint32_t adder, const Input::Event& forWhat,
                     std::function<void(const EventData&)> handler) {
   registered[(uint8_t)forWhat][adder] = handler;
}

void Input::callAll(const Input::Event& ev, const InputEventData& data) {
   EventData event = {std::type_index(typeid(Input)), data};
   for (auto& funcPair : registered[(uint8_t)ev])
      funcPair.second(event);
}

#pragma once
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "entt/entt.hpp"

using Entity = uint64_t;
using EntReg = entt::Registry<Entity>;

#include "Macros.hpp"

class Engine;
class Subsystem;

class Object {
  public:
   Object(Engine* engine);

   template <typename T>
   inline T* getSubsystem();

   virtual void update(float dt) = 0;
   virtual void sendEvents()     = 0;

  protected:
   Engine* engine;
   EntReg& reg;
};


class Subsystem : public Object {
  public:
   Subsystem(Engine* engine);
};

struct EventInfo {};

enum class EventType {
   Input,
   Update,
   PreRender,
   Render,
   PostRender,

   Custom
};

using EventHandler = std::function<void(const EventInfo&)>*;

class Engine {
  public:
   Engine() : SceneNodeProto{reg} {}


   template <typename T>
   void addSubsystem(const std::optional<std::string>& subName = {}) {
      subsystems[typeindex(T)] = std::make_unique<T>(this);
      if (subName)
         subsystemNames[subName.value()] = typeindex(T);
   }

   template <typename T>
   inline T* getSubsystem() {
      return static_cast<T*>(subsystems[typeindex(T)].get());
   }
   // inline Subsystem* getSubsystem(const std::string& name) { return subsystems[subsystemNames[name]].get(); }

   EntReg& getReg() { return reg; }

   bool update(float dt) {
      for (auto& system : subsystems)
         system.second->update(dt);

      for (auto& eventPair : events)

         return true;
   }

  private:
   entt::Prototype<Entity> SceneNodeProto;
   EntReg                  reg;

   std::vector < std::unordered_map<std::string, std::type_index>  subsystemNames;
   std::unordered_map<std::type_index, std::unique_ptr<Subsystem>> subsystems;
};


template <typename T>
inline T* Object::getSubsystem() {
   return engine->getSubsystem<T>();
}
// inline Subsystem* Object::getSubsystem(const std::string& name) { return engine->getSubsystem(name); }

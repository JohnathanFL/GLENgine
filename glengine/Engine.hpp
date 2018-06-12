#pragma once
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "Macros.hpp"

class Engine;
class Subsystem;

class Object {
  public:
   Object(Engine* engine) { this->engine = engine; }

   template <typename T>
   inline T* getSubsystem();

   inline Subsystem* getSubsystem(const std::string& name);

  protected:
   Engine* engine;
};


class Subsystem : public Object {
  public:
   Subsystem(Engine* engine) : Object(engine) {}
};

class Engine {
  public:
   template <typename T>
   void addSubsystem(const std::optional<std::string&> subName = {}) {
      subsystems[typeindex(T)] = std::make_unique<T>(this);
      if (subName)
         subsystemNames[subName.value()] = typeindex(T);
   }

   template <typename T>
   inline void getSubsystem() {
      return static_cast<T*>(subsystems[typeindex(T)].get());
   }
   inline Subsystem* getSubsystem(const std::string& name) { return subsystems[subsystemNames[name]].get(); }


  private:
   std::unordered_map<std::string, std::type_index>                subsystemNames;
   std::unordered_map<std::type_index, std::unique_ptr<Subsystem>> subsystems;
};


template <typename T>
inline T* Object::getSubsystem() {
   return engine->getSubsystem<T>();
}
inline Subsystem* Object::getSubsystem(const std::string& name) { return engine->getSubsystem(name); }

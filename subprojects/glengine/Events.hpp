#pragma once
#include <typeindex>

struct CustomEventData {
  public:
   CustomEventData() {}
   virtual ~CustomEventData() {}
};

struct EventData {
   std::type_index        sender;  // type_index{typeid(T)};
   const CustomEventData& data;
};

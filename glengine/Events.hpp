#pragma once
#include <typeindex>

struct CustomEventData {
  public:
   CustomEventData() {
   }
   virtual ~CustomEventData() {
   }
};

struct EventData {
   std::type_index        sender;
   const CustomEventData& data;
};

#pragma once
#include "VulkanBase.hpp"

// TODO: Memory pooling, rather than a new pool for each buff
struct GpuBuffer : VulkanObject {
   void*            data;
   vk::Buffer       buff;
   vk::DeviceMemory mem;

   size_t size;

   GpuBuffer() {}
   GpuBuffer(const GpuBuffer& rhs) : VulkanObject{rhs.dev} {}

   inline bool isValid() {
      if (dev == NullDevice)
         return false;
      // Todo: More checks


      return true;
   }
   //  Makes some code cleaner
   inline bool isInvalid() { return !isValid(); }

   inline bool isMapped() { return data != nullptr; }
   void        map() {
      if (isMapped())
         return;

      // No flags to even use yet...
      vk::MemoryMapFlags flags(0);

      data = dev.mapMemory(mem, 0, size, flags);
   }

   // Todo: overload for vector/array types, to make it easier
   template <typename T>
   void upload(T* mem, size_t num) {
      if (!isMapped())
         map();

      memcpy(data, mem, num * sizeof(T));
   }
};

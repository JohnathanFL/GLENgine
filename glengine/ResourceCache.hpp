#pragma once
#include <filesystem>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

// Notes: Hierarchical resources?

// C++ could really use this as a standard op.
#define typeindex(T) std::type_index(typeid(T))

using byte = uint8_t;

// Note: Maybe only refer to these by index, since they're likely to be accessed fairly rarely?

class Resource {
  public:
   const std::vector<std::string>    fileTypes;                              // The filetypes this Resource can load.
   virtual std::shared_ptr<Resource> load(const std::string& filePath) = 0;  // Loads the file at filePath into a new
                                                                             // Resource of the same type and returns it
};


class ResourceCache {
  public:
   ResourceCache();

   Resource* getResource(const std::string& res) { return nullptr; }

   void addNewDir(const std::string& dir) {
      for (auto& cache : caches)
         cache.second->addAllResources(dir);

      resourceSources.push_back(dir);
   }

   void addNewFile(const std::string& dir) {
      for (auto& cache : caches)
         cache.second->addAllResources(dir);

      resourceSources.push_back(dir);
   }


   std::vector<std::string> singleFileResources;                          /// Resources not grouped into any directory
   std::vector<std::string> resourceSources;                              /// Resources grouped into directories
   std::unordered_map<std::string, std::unique_ptr<Resource>> templates;  /// Which caches can handle which filetypes.
   std::unordered_map<std::type_index, std::vector<Resource*>*>;
   std::unordered_map<std::string, Resource*> caches;
};

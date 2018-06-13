#pragma once
#include <filesystem>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

// Notes: Hierarchical resources?

// C++ could really use this as a standard op.
#define typeindex(T) std::type_index(typeid(T))

using byte = uint8_t;

// Note: Maybe only refer to these by index, since they're likely to be accessed fairly rarely?

class Resource {
  public:
   virtual ~Resource();                                                   // Silence warnings
   virtual std::shared_ptr<Resource> load(const fs::path& filePath) = 0;  // Loads the file at filePath into a new
                                                                          // Resource of the same type and returns it.
                                                                          // Basically a factory method.

   const std::vector<std::string> fileTypes;  // The filetypes this Resource can load.
};

// Resource loading is handled based on their extensions.
// Resource fetching is done based on the path within the folders that were specifically loaded (addNewDir), or through
// "/" + the single filename (addNewFile)
class ResourceCache {
  public:
   ResourceCache();

   // Perhaps make this return tmplate so the user can add some special data for a factory?
   template <typename T>
   void registerResource(const std::vector<std::string>& extensions) {
      std::shared_ptr<Resource> tmplate = std::make_shared<T>();
      for (auto& ext : extensions)
         templates[ext] = tmplate;
   }

   std::shared_ptr<Resource> getResource(const std::string& res) { return resources[res]; }

   void addNewDir(const fs::path& dir) {
      resourceSources.push_back(dir);
      for (const fs::directory_entry& file : fs::recursive_directory_iterator(dir)) {
         if (!file.is_regular_file())
            continue;

         std::string fileKey = file.path().native().substr(dir.native().size() - 1),  // Remove the preceeding path
             ext             = file.path().extension();

         resources[fileKey] = templates[ext]->load(file);
      }
   }

   void addNewFile(const fs::path& path) {
      singleFileResources.push_back(path);

      std::string ext                           = path.extension();
      resources["/" + path.filename().native()] = resources[ext]->load(path);
   }


   std::vector<fs::path> singleFileResources;                             /// Resources not grouped into any directory
   std::vector<fs::path> resourceSources;                                 /// Resources grouped into directories
   std::unordered_map<std::string, std::shared_ptr<Resource>> templates;  /// Which caches can handle which filetypes.
   std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
};

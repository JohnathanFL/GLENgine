#pragma once
#include <fstream>
#include <string>

#include "json.hpp"
using json = nlohmann::json;

class Settings {
  public:
   Settings(const std::string& fileName) {
      std::ifstream settingsFile(fileName);
      settingsFile >> settings;
      settingsFile.close();
   }

   json::reference section(const std::string& sectionIndex) { return settings[sectionIndex]; }

  private:
   json settings;
};

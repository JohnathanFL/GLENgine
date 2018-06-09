#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <SDL2/SDL.h>

// The only kind of singleton allowed in any sane code
class Logger {
   static std::unique_ptr<Logger> instance;
   std::ofstream                  logFile;


  public:
   ~Logger() { logFile.close(); }

   static void Write(const std::string& from, const std::string& message) {
      Write(from, message.c_str());
   }

   static void Write(const std::string& from, const char* message) {
      const std::string overallMessage =
          "[" + from + "]@" + std::to_string(SDL_GetTicks() / 1000.0f) +
          "s: " + message + '\n';

      if (instance->logFile.is_open())
         instance->logFile << overallMessage;

      std::cout << overallMessage;
   }

   static void SetLogFile(const std::string& fileName) {
      if (instance->logFile.is_open())
         instance->logFile.close();

      instance->logFile.open(fileName);
   }
};

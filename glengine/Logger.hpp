#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <SDL2/SDL.h>

// The only kind of singleton allowed in any sane code
class Logger {
   static std::unique_ptr<Logger> instance;
   std::ofstream                  logFile;


   template <typename T, typename... Args>
   static void addTo(std::stringstream& stream, const T& arg,
                     const Args&... args) {
      stream << arg;
      addTo(stream, args...);
   }
   // Base case
   template <typename T>
   static void addTo(std::stringstream& stream, const T& arg) {
      stream << arg;
   }

   static void trim(std::string& str) {
      for (char& ch : str)
         if (ch == '\n')
            ch = ' ';
   }

  public:
   ~Logger() { logFile.close(); }

   template <typename... Args>
   static void Write(const std::string& from, const Args&... args) {
      std::stringstream messageBuilder;
      messageBuilder << "[" << from << "]@"
                     << std::to_string(SDL_GetTicks() / 1000.0f) << "s: ";

      addTo(messageBuilder, args...);


      std::string overallMessage = messageBuilder.str();
      trim(overallMessage);
      overallMessage += '\n';

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

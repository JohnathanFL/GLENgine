#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <SDL2/SDL.h>

// The only kind of singleton allowed in any sane code
class Logger {
  public:
   ~Logger() { logFile.close(); }


   /// Generalized writing function. Prints in [TYPE]@<time in secs>: <ARGS...>
   template <typename... Args>
   static void Write(const std::string& type, const Args&... args) {
      std::string message = buildStr(makeHeader(type), args...);

      if (instance->logFile.is_open())
         instance->logFile << message << std::endl << std::flush;

      std::cout << message << std::endl << std::flush;
   }

   // Shorthand write functions

   template <typename... Args>
   static void ErrorOut(const Args&... args) {
      Write("ERROR", args...);
      exit(-1);
   }

   template <typename... Args>
   static void Info(const Args&... args) {
      Write("INFO", args...);
   }

   /// Changes what file we write to when we log things.
   static void SetLogFile(const std::string& fileName) {
      if (instance->logFile.is_open())
         instance->logFile.close();

      instance->logFile.open(fileName);
   }

  private:
   static std::unique_ptr<Logger> instance;
   std::ofstream                  logFile;

   // Helper funcs

   /// Inputs a list of variadic args into a stringstream
   template <typename T, typename... Args>
   static void addTo(std::stringstream& stream, const T& arg, const Args&... args) {
      stream << arg;
      addTo(stream, args...);
   }
   // Base case
   template <typename T>
   static void addTo(std::stringstream& stream, const T& arg) {
      stream << arg;
   }

   /// Turns any newlines into spaces
   static void trim(std::string& str) {
      for (char& ch : str)
         if (ch == '\n')
            ch = ' ';
   }

   /// Turns a list of variadic args into a string
   template <typename... Args>
   static std::string buildStr(const Args&... args) {
      std::stringstream messageBuilder;
      addTo(messageBuilder, args...);
      std::string finalMessage = messageBuilder.str();
      trim(finalMessage);
      return finalMessage;
   }

   /// Make something like [VULKAN]@0.1231s
   static std::string makeHeader(const std::string& header) {
      return buildStr("[", header, "]@", SDL_GetTicks() / 1000.0, "s: ");
   }
};

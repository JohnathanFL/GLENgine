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

   template <typename... Args>
   static std::string buildStr(const Args&... args) {
      std::stringstream messageBuilder;
      addTo(messageBuilder, args...);
      std::string finalMessage = messageBuilder.str();
      trim(finalMessage);
      return finalMessage;
   }

   static std::string makeHeader(const std::string& header) {
      return buildStr("[", header, "]@", SDL_GetTicks() / 1000.0, ": ");
   }

  public:
   ~Logger() { logFile.close(); }


   template <typename... Args>
   static void Write(const std::string& type, const Args&... args) {
      std::string message = buildStr(makeHeader(type), args...);

      if (instance->logFile.is_open())
         instance->logFile << message << std::endl;

      std::cout << message << std::endl;
   }

   template <typename... Args>
   static void ErrorOut(const Args&... args) {
      Write("ERROR", args...);
      exit(-1);
   }

   static void SetLogFile(const std::string& fileName) {
      if (instance->logFile.is_open())
         instance->logFile.close();

      instance->logFile.open(fileName);
   }
};

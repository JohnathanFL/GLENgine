#!/bin/env python3

import os, xml.etree.ElementTree, sys

assert(len(sys.argv) == 2) # Only call with a source file

castXmlArgs = " -cxx-isystem $VULKAN_SDK/include -std=c++17 --castxml-output=1"
fileName = sys.argv[1]

os.system("castxml " + fileName + castXmlArgs + " -o " + fileName + ".ast.xml")

ast = xml.etree.ElementTree.parse(fileName + ".ast.xml").getroot()

mapFile = open(fileName + "-enummap.cpp", 'w')
mapFile.write("#pragma once\n#include <unordered_map>\n#include <" + fileName + ">\n")

for enum in ast.findall('Enumeration'):
    if enum.get("attributes") != "annotate(Stringify)":
        continue
    name = enum.get("name")
    mapFile.write("static const std::unordered_map<" + name + ", std::string> " + name + "Mappings = {")
    for val in enum:
        value = val.get("name")
        mapFile.write("{" + name + "::" + value + ", \"" + value + "\"},\n")
    mapFile.write('};\n')
    mapFile.write("""
    inline const std::string& to_string(const """ + name + """ from) { return """ + name + """Mappings.at(from); } 
   template <>                                                                                 
   inline """ + name +""" from_string<"""+ name + """>(const std::string& in) {                              
      for (const auto& pairing : """ + name + """Mappings)                                                 
         if (pairing.second == in)                                                             
            return pairing.first;                                                              
   }
    """)

os.system("rm " + fileName + ".ast.xml")


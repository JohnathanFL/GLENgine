#pragma once
#include <unordered_map>
#include <Pipeline.hpp>
static const std::unordered_map<Topology, std::string> TopologyMappings = {{Topology::Points, "Points"},
{Topology::Lines, "Lines"},
{Topology::LineStrips, "LineStrips"},
{Topology::Triangles, "Triangles"},
{Topology::TriangleStrips, "TriangleStrips"},
{Topology::TriangleFans, "TriangleFans"},
{Topology::LinesAdj, "LinesAdj"},
{Topology::LineStripsAdj, "LineStripsAdj"},
{Topology::TrianglesAdj, "TrianglesAdj"},
{Topology::TriangleStripsAdj, "TriangleStripsAdj"},
{Topology::PatchList, "PatchList"},
};

    inline const std::string& to_string(const Topology from) { return TopologyMappings.at(from); } 
   template <>                                                                                 
   inline Topology from_string<Topology>(const std::string& in) {                              
      for (const auto& pairing : TopologyMappings)                                                 
         if (pairing.second == in)                                                             
            return pairing.first;                                                              
   }
    static const std::unordered_map<FillMode, std::string> FillModeMappings = {{FillMode::Fill, "Fill"},
{FillMode::Line, "Line"},
{FillMode::Point, "Point"},
{FillMode::RectangleNV, "RectangleNV"},
};

    inline const std::string& to_string(const FillMode from) { return FillModeMappings.at(from); } 
   template <>                                                                                 
   inline FillMode from_string<FillMode>(const std::string& in) {                              
      for (const auto& pairing : FillModeMappings)                                                 
         if (pairing.second == in)                                                             
            return pairing.first;                                                              
   }
    static const std::unordered_map<CullMode, std::string> CullModeMappings = {{CullMode::None, "None"},
{CullMode::Front, "Front"},
{CullMode::Back, "Back"},
{CullMode::Both, "Both"},
};

    inline const std::string& to_string(const CullMode from) { return CullModeMappings.at(from); } 
   template <>                                                                                 
   inline CullMode from_string<CullMode>(const std::string& in) {                              
      for (const auto& pairing : CullModeMappings)                                                 
         if (pairing.second == in)                                                             
            return pairing.first;                                                              
   }
    static const std::unordered_map<CompareOp, std::string> CompareOpMappings = {{CompareOp::Never, "Never"},
{CompareOp::Less, "Less"},
{CompareOp::Equal, "Equal"},
{CompareOp::LessOrEqual, "LessOrEqual"},
{CompareOp::Greater, "Greater"},
{CompareOp::NotEqual, "NotEqual"},
{CompareOp::GreaterOrEqual, "GreaterOrEqual"},
{CompareOp::Always, "Always"},
};

    inline const std::string& to_string(const CompareOp from) { return CompareOpMappings.at(from); } 
   template <>                                                                                 
   inline CompareOp from_string<CompareOp>(const std::string& in) {                              
      for (const auto& pairing : CompareOpMappings)                                                 
         if (pairing.second == in)                                                             
            return pairing.first;                                                              
   }
    static const std::unordered_map<StencilOp, std::string> StencilOpMappings = {{StencilOp::Keep, "Keep"},
{StencilOp::Zero, "Zero"},
{StencilOp::Replace, "Replace"},
{StencilOp::IncrementAndClamp, "IncrementAndClamp"},
{StencilOp::DecrementAndClamp, "DecrementAndClamp"},
{StencilOp::Invert, "Invert"},
{StencilOp::IncrementAndWrap, "IncrementAndWrap"},
{StencilOp::DecrementAndWrap, "DecrementAndWrap"},
};

    inline const std::string& to_string(const StencilOp from) { return StencilOpMappings.at(from); } 
   template <>                                                                                 
   inline StencilOp from_string<StencilOp>(const std::string& in) {                              
      for (const auto& pairing : StencilOpMappings)                                                 
         if (pairing.second == in)                                                             
            return pairing.first;                                                              
   }
    
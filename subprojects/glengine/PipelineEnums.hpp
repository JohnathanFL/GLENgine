#pragma once

#include <vulkan/vulkan.h>

#include <EnumMap.hpp>

// I'd say there are enough to justify their own file, not to mention it cuts down on enum map build times.

// I'm remaking all these enums instead of using vulkan-hpp versions because I won't be using the 'e' prefixing in my
// code, and so want to stay consistent.
enum class STRINGIFY Topology : VkFlags {
   Points            = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
   Lines             = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
   LineStrips        = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
   Triangles         = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
   TriangleStrips    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
   TriangleFans      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
   LinesAdj          = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
   LineStripsAdj     = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
   TrianglesAdj      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
   TriangleStripsAdj = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
   PatchList         = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
};

enum class STRINGIFY FillMode : VkFlags {
   Fill        = VK_POLYGON_MODE_FILL,
   Line        = VK_POLYGON_MODE_LINE,
   Point       = VK_POLYGON_MODE_POINT,
   RectangleNV = VK_POLYGON_MODE_FILL_RECTANGLE_NV,
};


enum class STRINGIFY CullMode : VkFlags {
   None  = VK_CULL_MODE_NONE,
   Front = VK_CULL_MODE_FRONT_BIT,
   Back  = VK_CULL_MODE_BACK_BIT,
   Both  = VK_CULL_MODE_FRONT_AND_BACK,
};

// Back to Physics, baby
enum class STRINGIFY FrontFaceRule : VkFlags {
   RightHand = VK_FRONT_FACE_COUNTER_CLOCKWISE,
   LeftHand  = VK_FRONT_FACE_CLOCKWISE,
};


enum class STRINGIFY CompareOp {
   Never          = VK_COMPARE_OP_NEVER,
   Less           = VK_COMPARE_OP_LESS,
   Equal          = VK_COMPARE_OP_EQUAL,
   LessOrEqual    = VK_COMPARE_OP_LESS_OR_EQUAL,
   Greater        = VK_COMPARE_OP_GREATER,
   NotEqual       = VK_COMPARE_OP_NOT_EQUAL,
   GreaterOrEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
   Always         = VK_COMPARE_OP_ALWAYS
};


enum class STRINGIFY StencilOp {
   Keep              = VK_STENCIL_OP_KEEP,
   Zero              = VK_STENCIL_OP_ZERO,
   Replace           = VK_STENCIL_OP_REPLACE,
   IncrementAndClamp = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
   DecrementAndClamp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
   Invert            = VK_STENCIL_OP_INVERT,
   IncrementAndWrap  = VK_STENCIL_OP_INCREMENT_AND_WRAP,
   DecrementAndWrap  = VK_STENCIL_OP_DECREMENT_AND_WRAP
};

enum class STRINGIFY LogicOp {
   Clear        = VK_LOGIC_OP_CLEAR,
   And          = VK_LOGIC_OP_AND,
   AndReverse   = VK_LOGIC_OP_AND_REVERSE,
   Copy         = VK_LOGIC_OP_COPY,
   AndInverted  = VK_LOGIC_OP_AND_INVERTED,
   NoOp         = VK_LOGIC_OP_NO_OP,
   Xor          = VK_LOGIC_OP_XOR,
   Or           = VK_LOGIC_OP_OR,
   Nor          = VK_LOGIC_OP_NOR,
   Equivalent   = VK_LOGIC_OP_EQUIVALENT,
   Invert       = VK_LOGIC_OP_INVERT,
   OrReverse    = VK_LOGIC_OP_OR_REVERSE,
   CopyInverted = VK_LOGIC_OP_COPY_INVERTED,
   OrInverted   = VK_LOGIC_OP_OR_INVERTED,
   Nand         = VK_LOGIC_OP_NAND,
   Set          = VK_LOGIC_OP_SET
};


enum class BlendFactor {
   Zero                  = VK_BLEND_FACTOR_ZERO,
   One                   = VK_BLEND_FACTOR_ONE,
   SrcColor              = VK_BLEND_FACTOR_SRC_COLOR,
   OneMinusSrcColor      = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
   DstColor              = VK_BLEND_FACTOR_DST_COLOR,
   OneMinusDstColor      = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
   SrcAlpha              = VK_BLEND_FACTOR_SRC_ALPHA,
   OneMinusSrcAlpha      = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
   DstAlpha              = VK_BLEND_FACTOR_DST_ALPHA,
   OneMinusDstAlpha      = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
   ConstantColor         = VK_BLEND_FACTOR_CONSTANT_COLOR,
   OneMinusConstantColor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
   ConstantAlpha         = VK_BLEND_FACTOR_CONSTANT_ALPHA,
   OneMinusConstantAlpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
   SrcAlphaSaturate      = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
   Src1Color             = VK_BLEND_FACTOR_SRC1_COLOR,
   OneMinusSrc1Color     = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
   Src1Alpha             = VK_BLEND_FACTOR_SRC1_ALPHA,
   OneMinusSrc1Alpha     = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA
};

enum class BlendOp {
   Add                 = VK_BLEND_OP_ADD,
   Subtract            = VK_BLEND_OP_SUBTRACT,
   ReverseSubtract     = VK_BLEND_OP_REVERSE_SUBTRACT,
   Min                 = VK_BLEND_OP_MIN,
   Max                 = VK_BLEND_OP_MAX,
   ZeroEXT             = VK_BLEND_OP_ZERO_EXT,
   SrcEXT              = VK_BLEND_OP_SRC_EXT,
   DstEXT              = VK_BLEND_OP_DST_EXT,
   SrcOverEXT          = VK_BLEND_OP_SRC_OVER_EXT,
   DstOverEXT          = VK_BLEND_OP_DST_OVER_EXT,
   SrcInEXT            = VK_BLEND_OP_SRC_IN_EXT,
   DstInEXT            = VK_BLEND_OP_DST_IN_EXT,
   SrcOutEXT           = VK_BLEND_OP_SRC_OUT_EXT,
   DstOutEXT           = VK_BLEND_OP_DST_OUT_EXT,
   SrcAtopEXT          = VK_BLEND_OP_SRC_ATOP_EXT,
   DstAtopEXT          = VK_BLEND_OP_DST_ATOP_EXT,
   XorEXT              = VK_BLEND_OP_XOR_EXT,
   MultiplyEXT         = VK_BLEND_OP_MULTIPLY_EXT,
   ScreenEXT           = VK_BLEND_OP_SCREEN_EXT,
   OverlayEXT          = VK_BLEND_OP_OVERLAY_EXT,
   DarkenEXT           = VK_BLEND_OP_DARKEN_EXT,
   LightenEXT          = VK_BLEND_OP_LIGHTEN_EXT,
   ColordodgeEXT       = VK_BLEND_OP_COLORDODGE_EXT,
   ColorburnEXT        = VK_BLEND_OP_COLORBURN_EXT,
   HardlightEXT        = VK_BLEND_OP_HARDLIGHT_EXT,
   SoftlightEXT        = VK_BLEND_OP_SOFTLIGHT_EXT,
   DifferenceEXT       = VK_BLEND_OP_DIFFERENCE_EXT,
   ExclusionEXT        = VK_BLEND_OP_EXCLUSION_EXT,
   InvertEXT           = VK_BLEND_OP_INVERT_EXT,
   InvertRgbEXT        = VK_BLEND_OP_INVERT_RGB_EXT,
   LineardodgeEXT      = VK_BLEND_OP_LINEARDODGE_EXT,
   LinearburnEXT       = VK_BLEND_OP_LINEARBURN_EXT,
   VividlightEXT       = VK_BLEND_OP_VIVIDLIGHT_EXT,
   LinearlightEXT      = VK_BLEND_OP_LINEARLIGHT_EXT,
   PinlightEXT         = VK_BLEND_OP_PINLIGHT_EXT,
   HardmixEXT          = VK_BLEND_OP_HARDMIX_EXT,
   HslHueEXT           = VK_BLEND_OP_HSL_HUE_EXT,
   HslSaturationEXT    = VK_BLEND_OP_HSL_SATURATION_EXT,
   HslColorEXT         = VK_BLEND_OP_HSL_COLOR_EXT,
   HslLuminosityEXT    = VK_BLEND_OP_HSL_LUMINOSITY_EXT,
   PlusEXT             = VK_BLEND_OP_PLUS_EXT,
   PlusClampedEXT      = VK_BLEND_OP_PLUS_CLAMPED_EXT,
   PlusClampedAlphaEXT = VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT,
   PlusDarkerEXT       = VK_BLEND_OP_PLUS_DARKER_EXT,
   MinusEXT            = VK_BLEND_OP_MINUS_EXT,
   MinusClampedEXT     = VK_BLEND_OP_MINUS_CLAMPED_EXT,
   ContrastEXT         = VK_BLEND_OP_CONTRAST_EXT,
   InvertOvgEXT        = VK_BLEND_OP_INVERT_OVG_EXT,
   RedEXT              = VK_BLEND_OP_RED_EXT,
   GreenEXT            = VK_BLEND_OP_GREEN_EXT,
   BlueEXT             = VK_BLEND_OP_BLUE_EXT
};

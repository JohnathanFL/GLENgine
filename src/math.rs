

pub use cgmath as cg;


pub use cgmath;
pub use cgmath::{vec1, vec2, vec3, vec4};

pub type Float = f32;
pub type Int = i32;
pub type Unsigned = u32;

pub type Transformation = cg::Transform3<Float>;

// Trying to mostly emulate GLM types
pub type Mat4x4 = cg::Matrix4<Float>;
pub type Quat = cg::Quaternion<Float>;
pub type Vec3 = cg::Vector3<Float>;
pub type Vec2 = cg::Vector2<Float>;
pub type IVec3 = cg::Vector3<Int>;
pub type IVec2 = cg::Vector2<Int>;
pub type UVec3 = cg::Vector3<Unsigned>;
pub type UVec2 = cg::Vector3<Unsigned>;
pub type Point3 = cg::Point3<f32>;
pub type Perspective = cg::Perspective<Float>;
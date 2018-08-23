use std::cmp::Ordering;
use cgmath::prelude::Array;
use num_traits::Num;

pub use cgmath as cg;


pub use cgmath::{self, Transform, Rad, Deg};
pub use cgmath::{SquareMatrix, prelude, prelude::*};
pub use cgmath::{vec1, vec2, vec3, vec4};

pub type Float = f32;
pub type Int = i32;
pub type Unsigned = u32;

// Trying to mostly emulate GLM types
pub type Mat4x4 = cg::Matrix4<Float>;
pub type Quat = cg::Quaternion<Float>;
pub type Vec3 = cg::Vector3<Float>;
pub type Vec2 = cg::Vector2<Float>;
pub type IVec3 = cg::Vector3<Int>;
pub type IVec2 = cg::Vector2<Int>;
pub type UVec3 = cg::Vector3<Unsigned>;
pub type UVec2 = cg::Vector2<Unsigned>;
pub type Point3 = cg::Point3<f32>;
pub type Perspective = cg::Perspective<Float>;


#[inline]
pub fn rel_to_range<T>(x: T, min: T, max: T) -> Ordering
    where T: Ord + Copy {
    if x < min {
        return Ordering::Less;
    } else if x > max {
        return Ordering::Greater;
    } else {
        return Ordering::Equal;
    }
}

#[inline]
pub fn is_in_range<T>(x: T, min: T, max: T) -> bool
    where T: Ord + Copy {
    return match rel_to_range(x, min, max) {
        Ordering::Less | Ordering::Greater => false,
        Ordering::Equal => true
    };
}

pub fn clamp<T>(x: T, min: T, max: T) -> T
    where T: Ord + Copy {
    return match rel_to_range(x, min, max) {
        Ordering::Less => min,
        Ordering::Greater => max,
        Ordering::Equal => x
    };
}

pub fn clamp_components<T>(x: T, min: T, max: T) -> T
    where T: Array + Copy, T::Element: Ord {
    let mut x = x;
    for i in 0..=T::len() {
        x[i] = clamp(x[i], min[i], max[i]);
    }

    x
}
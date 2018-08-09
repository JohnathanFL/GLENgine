use specs::{Component, storage::{VecStorage, NullStorage, DenseVecStorage}, Entity};
use specs_derive;

use smallvec::SmallVec;


use math::*;

#[derive(Debug, Copy, Clone)]
pub struct DeltaTime(f64);

pub mod transform {
    use super::*;

    #[derive(Debug, Copy, Clone, Component)]
    #[storage(VecStorage)]
    pub struct Pos(pub Vec3);

    #[derive(Debug, Copy, Clone, Component)]
    #[storage(VecStorage)]
    pub struct Rot(pub Quat);

    #[derive(Debug, Copy, Clone, Component)]
    #[storage(VecStorage)]
    pub struct Scale(pub Vec3);

    #[derive(Debug, Copy, Clone, Component)]
    #[storage(VecStorage)]
    pub enum CachedTransform {
        Dirty,
        Cached(Mat4x4),
    }

    impl CachedTransform {
        pub fn calc(&mut self, pos: Vec3, rot: Quat, scale: Vec3) {
            *self = CachedTransform::Cached(calc_transform(pos, rot, scale));
        }
    }

    pub fn calc_transform(pos: Vec3, rot: Quat, scale: Vec3) -> Mat4x4 {
        return
            Mat4x4::from_translation(pos)
                * Mat4x4::from(rot)
                * Mat4x4::from_nonuniform_scale(scale.x, scale.y, scale.z);
    }
}

pub mod hierarchy {
    use super::*;

    #[derive(Debug, Clone, Component, Default)]
    #[storage(NullStorage)]
    pub struct RootNode;

    #[derive(Debug, Clone, Component)]
    #[storage(VecStorage)]
    pub struct Parent(Entity);

    #[derive(Debug, Clone, Component)]
    #[storage(VecStorage)]
    pub struct Children(SmallVec<[Entity; 2]>);
}


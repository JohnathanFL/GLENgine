use std::ops::{Deref, DerefMut};
use std::mem::transmute;
use std::iter::FromIterator;

use specs::{
    self,
    Component, Entity, System, ReadStorage, WriteStorage, Join, Read, Builder, EntityBuilder,
    storage::{VecStorage, NullStorage, DenseVecStorage},
    world::{World, Bundle, Generation},
};
use specs_derive;

use smallvec::SmallVec;


use math::*;

#[derive(Debug, Copy, Clone, NewType)]
pub struct DeltaTime(f64);


/// All components are working in a LOCAL space.
/// The Root transform
pub mod transform {
    use super::*;

    #[derive(Debug, Copy, Clone, Component)]
    #[storage(VecStorage)]
    pub struct Transformation {
        pub pos: Vec3,
        pub rot: Quat,
        // /// TODO
        // pub scale: Vec3
    }

    impl Transformation {
        #[inline]
        pub fn origin() -> Transformation {
            return Transformation {
                pos: vec3(0.0, 0.0, 0.0),
                rot: Quat::from_angle_x(Rad(0.0)),
                // scale: vec3(1.0,1.0,1.0)
            };
        }

        #[inline]
        pub fn at(pos: Vec3, rot: Quat) -> Transformation {
            return Transformation {
                pos, rot
            };
        }
    }

    impl From<Transformation> for Mat4x4 {
        #[inline]
        fn from(transform: Transformation) -> Mat4x4 {
            let rotMat: Mat4x4 = transform.rot.into();

            return Mat4x4::from_translation(transform.pos) * rotMat * Mat4x4::from_scale(1.0);
        }
    }

    impl<'a> From<&'a Transformation> for Mat4x4 {
        #[inline]
        fn from(transform: &Transformation) -> Mat4x4 {
            return (*transform).into();
        }
    }

    #[derive(Debug, Copy, Clone, Component)]
    #[storage(VecStorage)]
    pub struct CachedTransform {
        pub dirty: bool,
        pub cached: Mat4x4
    }

    impl CachedTransform {
        #[inline]
        pub fn calc(&mut self, transform: &Transformation) {
            self.dirty = false;
            self.cached = transform.into();
        }

        #[inline]
        pub fn new() -> Self {
            return CachedTransform {
                dirty: true,
                cached: Mat4x4::identity()
            };
        }
    }

    pub struct TransformCalc;

    impl<'a> System<'a> for TransformCalc {
        type SystemData = (
            Read<'a, hierarchy::RootNode>,
            ReadStorage<'a, hierarchy::Node>,
            ReadStorage<'a, Transformation>,
            WriteStorage<'a, CachedTransform>);

        fn run(&mut self, data: Self::SystemData) {
            let (
                rootNode,
                nodes,
                transforms,
                mut cachedTransforms) = data;

            // Summary:
            // 1. Push root onto the stack
            // 2. For each node on the stack:
            //      2.1 Calc transform, using it's parent's transform
            //      2.2 Push each of the node's children onto the stack, using the node's
            //          transform as the parent

            let mut rootNode: Entity = **rootNode;
            let mut identityTransform = Mat4x4::identity();

            let mut chain = vec![(identityTransform, rootNode)];

            while !chain.is_empty() {
                //println!("Calculating transforms!");
                let (parentTransform, curNode) = chain.pop().unwrap();
                let cachedTransform = cachedTransforms.get_mut(curNode)
                    .expect("Node had no cached transform!");
                let transform = transforms.get(curNode)
                    .expect("Node had no transform!");


                cachedTransform.calc(transform);

                for child in &nodes.get(curNode).expect("Node had no children!").children {
                    chain.push((cachedTransform.cached, *child));
                }
            }
        }
    }
}

pub mod hierarchy {
    use super::*;


    #[derive(Copy, Clone, NewType)]
    pub struct RootNode(Entity);

    impl Default for RootNode {
        fn default() -> RootNode {
            let newEnt: Entity = unsafe {
                // Forgive me rust-father, for I have sinned.
                // This should be safe since all worlds must have registered the SceneBundle in
                // order to use this stuff, and that automatically adds a root ent.
                transmute(0u64)
            };

            return newEnt.into();
        }
    }

    #[derive(Debug, Clone, Component)]
    pub struct Node {
        pub parent: Option<Entity>,
        pub children: SmallVec<[Entity; 2]>
    }

    impl Node {
        pub fn new(parent: Option<Entity>, children: &[Entity]) -> Node {
            return Node {
                parent,
                children: SmallVec::from_slice(children)
            };
        }
    }
}

pub trait NodeBuilder {
    /// Creates a new node with all base scene components
    fn new_node(&mut self, parent: Option<Entity>,
                transform: transform::Transformation) -> EntityBuilder;
}

impl NodeBuilder for World {
    fn new_node(&mut self, parent: Option<Entity>,
                transform: transform::Transformation) -> EntityBuilder {
        use self::{hierarchy::*, transform::*};
        return self.create_entity()
            .with(Transformation::origin())
            .with(CachedTransform::new())
            .with(Node::new(parent, &[]));
    }
}

pub struct SceneBundle;

impl Bundle for SceneBundle {
    fn add_to_world(self, world: &mut World) {
        use self::{hierarchy::*, transform::*};

        world.register::<Node>();
        world.register::<Transformation>();
        world.register::<CachedTransform>();

        let rootEnt = world.new_node(None, Transformation::origin()).build();

        world.add_resource(RootNode::from(rootEnt));
        world.add_resource(DeltaTime(0.0));
    }
}
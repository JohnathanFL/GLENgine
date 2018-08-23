pub use chunk::*;

use std::collections::{BTreeMap, btree_map::Values};

use std::cmp::Ordering;
use std::convert::From;

use specs::{Component, storage::DenseVecStorage};

use math::*;

use num_traits::PrimInt;

use specs::world::{Bundle, World};

#[derive(Debug, Copy, Clone)]
pub enum DrawStyle {
    ChunkByChunk,
    AllAtOnce,
}

#[derive(Debug, Clone, Component)]
#[storage(DenseVecStorage)]
pub struct Volume {
    style: DrawStyle,
    chunks: BTreeMap<PackedChunkCoords, Chunk>,
}

/// Since nalgebra::Vector3<usize> can't be used as a key (No Ord), make our own.
#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub struct PackedChunkCoords(u16);


impl<T> From<(T, T, T)> for PackedChunkCoords
    where T: Into<u8> + Copy {
    #[inline]
    fn from(f: (T, T, T)) -> PackedChunkCoords {
        let (x, y, z) = f;
        let mut res = z.into() as u8 as u16;
        res |= ((y.into() as u8 as u16) << 5);
        res |= ((x.into() as u8 as u16) << 10);

        return PackedChunkCoords(res);
    }
}

impl PackedChunkCoords {
    #[inline]
    fn unpack<T>(&self) -> (T, T, T)
        where T: From<u16>
    {
        // First 5 bits set
        const ZMASK: u16 = 0x1F;
        const YMASK: u16 = ZMASK << 5;
        const XMASK: u16 = YMASK << 5;

        return ((self.0 & XMASK).into(), (self.0 & YMASK).into(), (self.0 & ZMASK).into());
    }
}


impl Volume {
    pub fn new(style: DrawStyle) -> Volume {
        return Volume {
            style,
            chunks: BTreeMap::new(),
        };
    }

    /// Gets the voxel in the specified chunk at the coords.
    /// If there is no chunk at whichChunk, then it will always return Voxel(0).
    pub fn get_vox(&self, whichChunk: PackedChunkCoords, coords: UVec3) -> Voxel {
        match self.chunks.get(&whichChunk) {
            Some(chunk) => return chunk.get(coords),
            None => return AIR_VOXEL
        }
    }

    pub fn chunks(&self) -> impl Iterator<Item = &Chunk> {
        return self.chunks.values();
    }

    pub fn add_chunk(&mut self, coords: PackedChunkCoords) {
        if !self.chunks.contains_key(&coords) {
            self.chunks.insert(coords, Chunk::new());
        }
    }

    pub fn chunk(&self, coords: PackedChunkCoords) -> Option<&Chunk> {
        return self.chunks.get(&coords);
    }

    pub fn chunk_mut(&mut self, coords: PackedChunkCoords) -> Option<&mut Chunk> {
        return self.chunks.get_mut(&coords);
    }
}

pub struct VolumeBundle;

impl Bundle for VolumeBundle {
    fn add_to_world(self, world: &mut World) {
        world.register::<Volume>();
    }
}
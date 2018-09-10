use mesh::{VoxelMesh, VertVoxel, VertPos};
use math::*;


pub const CHUNK_SIZE: usize = 32;



pub type Voxel = u32;

pub const AIR_VOXEL: Voxel = 0;

pub type ChunkArray<T> = [[[T; CHUNK_SIZE]; CHUNK_SIZE]; CHUNK_SIZE];

/// To be used with frequently changing data.
#[derive(Debug, Clone)]
#[repr(C)]
pub struct Chunk {
    numUsed: usize, // Number of blocks currently in use. Known for allocating vertex buffers

    isDirty: bool,
    pub blocks: ChunkArray<Voxel>,
    // TODO: The blocks should be in CPU accessible and the mesh should be DeviceLocal for
    // compute shaders to work better.
}




impl Chunk {
    pub fn new() -> Chunk {
        return Chunk {
            numUsed: 0,
            isDirty: false, // No blocks == No possible mesh
            blocks: [[[0; 32]; 32]; 32],
        };
    }

    pub fn with_background(backgroundData: Voxel) -> Chunk {
        return Chunk {
            numUsed: 0,
            isDirty: false,
            blocks: [[[backgroundData; 32]; 32]; 32],
        };
    }

    #[inline]
    pub fn get(&self, coords: UVec3) -> Voxel {
        return self.blocks[coords[0] as usize][coords[1] as usize][coords[2] as usize];
    }

    #[inline]
    pub fn set(&mut self, coords: UVec3, newVal: Voxel) {
        self.blocks[coords[0] as usize][coords[1] as usize][coords[2] as usize] = newVal;
    }

    /// Brings the mesh storage down to exactly what is needed, and nothing more.
    /// Use only on rarely updated chunks.
    #[inline]
    pub fn trim_storage(&mut self) {
        self.mesh.shrink_to_fit();
    }
}
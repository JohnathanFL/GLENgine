use mesh::{VoxelMesh, VertVoxel, VertPos};
use math::*;


const CHUNK_SIZE: usize = 32;



pub type Voxel = u32;

pub const AIR_VOXEL: Voxel = 0;

pub type ChunkArray<T> = [[[T; CHUNK_SIZE]; CHUNK_SIZE]; CHUNK_SIZE];

pub type MesherFunc = Fn(&Chunk, Option<VoxelMesh>) -> VoxelMesh;

/// To be used with frequently changing data.
#[derive(Debug, Clone)]
#[repr(C)]
pub struct Chunk {
    numUsed: usize, // Number of blocks currently in use. Known for allocating vertex buffers

    isDirty: bool,
    blocks: ChunkArray<Voxel>,
    mesh: VoxelMesh
    // TODO: The blocks should be in CPU accessible and the mesh should be DeviceLocal for
    // compute shaders to work better.
}


pub fn data_mesher(chunk: &Chunk, oldMesh: Option<VoxelMesh>) -> VoxelMesh {
    /*

    1256 -> +Z
    3276 -> +X
    1243 -> +Y

       0--------------1
      /|             /|
     / |            / |
    3--+-----------2  |
    |  |           |  |
    |  |           |  |
    |  |           |  |
    |  4-----------+--5
    | /            | /
    |/             |/
    7--------------6

    */
    const DIST_TO_WALL: f32 = 1.0 / ((CHUNK_SIZE * 2) as f32);
    let VERTS: [Vec3; 8] = [
        vec3(-DIST_TO_WALL, DIST_TO_WALL, DIST_TO_WALL),   // 0
        vec3(DIST_TO_WALL, DIST_TO_WALL, DIST_TO_WALL),    // 1
        vec3(DIST_TO_WALL, DIST_TO_WALL, -DIST_TO_WALL),   // 2
        vec3(-DIST_TO_WALL, DIST_TO_WALL, -DIST_TO_WALL),  // 3
        vec3(-DIST_TO_WALL, -DIST_TO_WALL, DIST_TO_WALL),  // 4
        vec3(DIST_TO_WALL, -DIST_TO_WALL, DIST_TO_WALL),   // 5
        vec3(DIST_TO_WALL, -DIST_TO_WALL, -DIST_TO_WALL),  // 6
        vec3(-DIST_TO_WALL, -DIST_TO_WALL, -DIST_TO_WALL), // 7
    ];
    let mut res = oldMesh.unwrap_or(VoxelMesh::new());

    // 1 block is 1/32 of the chunk. Dist from center of block to wall of block is thus
    // (1/32)/2 = (1/64)


    for z in 0..CHUNK_SIZE {
        for y in 0..CHUNK_SIZE {
            for x in 0..CHUNK_SIZE {
                let midOfBlock = vec3(
                    ((x + 1) as f32) / CHUNK_SIZE as f32,
                    ((y + 1) as f32) / CHUNK_SIZE as f32,
                    ((z + 1) as f32) / CHUNK_SIZE as f32,
                );

                let voxel = chunk.blocks[x][y][z];

                /// Simple code-dupe reduction. Takes midOfBlock and res implicitly, as well as a
                /// list of 1+ indices (taking from the cube art above) to add
                macro_rules! vox_vertex {
                        ($index: expr) => {
                            res.push(((midOfBlock + VERTS[$index]).into(), voxel.into()));
                        };

                        ($($index: expr),*) => {
                            $(
                                vox_vertex!($index);
                            )*
                        };
                    }

                // Note: Each of the following invocations of vox_vertex is doing 1 full
                // quad, with each triangle using the right hand rule for which face.
                // Reserving before each macro to try and get a *little* extra performance.

                // Always make sure we already have enough space for the worst case scenario.
                // Should give at least a mild performance bump
                res.reserve(36);

                if x == (CHUNK_SIZE - 1) || chunk.blocks[x + 1][y][z] == 0 { // +X Quad
                    vox_vertex!(5,1,2,  5,2,6);
                }
                if x == 0 || chunk.blocks[x - 1][y][z] == 0 { // -X Quad
                    vox_vertex!(7,3,4,  4,3,0);
                }


                if y == (CHUNK_SIZE - 1) || chunk.blocks[x][y + 1][z] == 0 { // +Y Quad
                    vox_vertex!(2,1,0,  3,2,0);
                }
                if y == 0 || chunk.blocks[x][y - 1][z] == 0 {
                    vox_vertex!(7,4,5,  7,5,6);
                }


                if z == (CHUNK_SIZE - 1) || chunk.blocks[x][y][z + 1] == 0 { // +Z Quad
                    vox_vertex!(5,4,0,  1,5,0);
                }
                if z == 0 || chunk.blocks[x][y][z - 1] == 0 { // -Z Quad
                    vox_vertex!(6,2,3,  7,6,3);
                }
            }
        }
    }

    res
}

impl Chunk {
    pub fn new() -> Chunk {
        return Chunk {
            numUsed: 0,
            isDirty: false, // No blocks == No possible mesh
            blocks: [[[0; 32]; 32]; 32],
            mesh: VoxelMesh::new(),
        };
    }

    pub fn with_background(backgroundData: Voxel) -> Chunk {
        return Chunk {
            numUsed: 0,
            isDirty: false,
            blocks: [[[backgroundData; 32]; 32]; 32],
            mesh: VoxelMesh::new(),
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